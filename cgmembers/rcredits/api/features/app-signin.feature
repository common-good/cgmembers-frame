Feature: App Token Sign-in
AS a member using the CGPay store app
I WANT to sign in once and stay signed in on my phone
SO I do not type my password every time, and a lost or stolen phone can be cut off
# These are the server-to-server endpoints the node app calls for the store app (shared secret, never the browser).
# The app keeps only a token; the server keeps only its hash.

Setup:
  Given members:
  | uid  | fullName | phone  | email | cc  | flags  | pass | city | state |*
  | .ZPA | Abe One  | +20001 | a@    | ccA | ok     | Aa1  | Aton | AL    |
  | .ZPB | Bea Two  | +20002 | b@    | ccB | ok,mfa | Bb2  | Bton | MA    |
  | .ZPD | Dee Four | +20004 | d@    | ccD |        | Dd4  | Dton | DC    |
  And member ".ZPA" has "card" steps done: "all"
  And member ".ZPB" has "card" steps done: "all"

# Rule: The right password signs the app in with a token, and only the token's hash is kept.

Scenario: A member signs in to the app
  When cgpay posts "app-signin" with:
  | identifier | password | label  | deviceKey |*
  | .ZPA       | Aa1      | Pixel  | key1      |
  Then we reply "ok" with JSON values:
  | token | uid |*1
  | ?     | ?   |
  And count "app_tokens" is 1
  And these "app_tokens":
  | uid  | label | trusted |*
  | .ZPA | Pixel | 0       |

Scenario: A member signs in by email address
  When cgpay posts "app-signin" with:
  | identifier | password | label |*
  | a@         | Aa1      | Pixel |
  Then we reply "ok" with JSON values:
  | token |*1
  | ?     |

Scenario: A wrong password gets no token
  When cgpay posts "app-signin" with:
  | identifier | password | label |*
  | .ZPA       | wrong    | Pixel |
  Then we reply "unauth" with JSON values:
  | error     |*1
  | bad-login |
  And count "app_tokens" is 0

Scenario: An unknown account gets no token
  When cgpay posts "app-signin" with:
  | identifier | password | label |*
  | .ZPZ       | Aa1      | Pixel |
  Then we reply "unauth" with JSON values:
  | error     |*1
  | bad-login |
  And count "app_tokens" is 0

Scenario: A caller without the shared secret is refused
  When a stranger posts "app-signin" with:
  | identifier | password | label |*
  | .ZPA       | Aa1      | Pixel |
  Then we reply "unauth" with: "?"
  And count "app_tokens" is 0

# Rule: A member with multi-factor authentication must also give the passcode we email.

Scenario: A member with MFA is asked for the passcode
  Given next random code is "12345678"
  When cgpay posts "app-signin" with:
  | identifier | password | label | deviceKey |*
  | .ZPB       | Bb2      | Pixel | key2      |
  Then we reply "ok" with JSON values:
  | mfa |*1
  | ?   |
  And we email "confirm-signin" to member "b@" with subs:
  | nonce    |*
  | 12345678 |
  And count "app_tokens" is 0

Scenario: The right passcode signs the app in
  Given next random code is "12345678"
  And cgpay posts "app-signin" with:
  | identifier | password | label | deviceKey |*
  | .ZPB       | Bb2      | Pixel | key2      |
  And reply "mfa" is kept as var "mfa"
  When cgpay posts "app-mfa" with:
  | mfa  | code     | trust |*
  | %mfa | 12345678 | 0     |
  Then we reply "ok" with JSON values:
  | token |*1
  | ?     |
  And these "app_tokens":
  | uid  | label | trusted |*
  | .ZPB | Pixel | 0       |

Scenario: A wrong passcode gets no token
  Given next random code is "12345678"
  And cgpay posts "app-signin" with:
  | identifier | password | label | deviceKey |*
  | .ZPB       | Bb2      | Pixel | key2      |
  And reply "mfa" is kept as var "mfa"
  When cgpay posts "app-mfa" with:
  | mfa  | code     | trust |*
  | %mfa | 99999999 | 0     |
  Then we reply "unauth" with JSON values:
  | error    |*1
  | bad-code |
  And count "app_tokens" is 0

# Rule: A device the member trusts skips the passcode next time, even after signing out.

Scenario: A trusted device signs in without a passcode
  Given next random code is "12345678"
  And cgpay posts "app-signin" with:
  | identifier | password | label | deviceKey |*
  | .ZPB       | Bb2      | Pixel | key2      |
  And reply "mfa" is kept as var "mfa"
  And cgpay posts "app-mfa" with:
  | mfa  | code     | trust |*
  | %mfa | 12345678 | 1     |
  And reply "token" is kept as var "token"
  And cgpay posts "app-revoke" with:
  | token  |*
  | %token |
  When cgpay posts "app-signin" with:
  | identifier | password | label | deviceKey |*
  | .ZPB       | Bb2      | Pixel | key2      |
  Then we reply "ok" with JSON values:
  | token |*1
  | ?     |
  And count "app_tokens" is 2

Scenario: Another device is not trusted
  Given next random code is "12345678"
  And cgpay posts "app-signin" with:
  | identifier | password | label | deviceKey |*
  | .ZPB       | Bb2      | Pixel | key2      |
  And reply "mfa" is kept as var "mfa"
  And cgpay posts "app-mfa" with:
  | mfa  | code     | trust |*
  | %mfa | 12345678 | 1     |
  When cgpay posts "app-signin" with:
  | identifier | password | label | deviceKey |*
  | .ZPB       | Bb2      | Tablet | key3  |
  Then we reply "ok" with JSON values:
  | mfa |*1
  | ?   |

# Rule: A token gives the device credentials until it is revoked.

Scenario: The app asks for its device credentials
  Given cgpay posts "app-signin" with:
  | identifier | password | label |*
  | .ZPA       | Aa1      | Pixel |
  And reply "token" is kept as var "token"
  When cgpay posts "app-device" with:
  | token  |*
  | %token |
  Then we reply "ok" with JSON values:
  | uid |*1
  | ?   |

Scenario: A made-up token is refused
  When cgpay posts "app-device" with:
  | token |*
  | abc   |
  Then we reply "unauth" with JSON values:
  | error     |*1
  | bad-token |

Scenario: A signed-out token no longer works
  Given cgpay posts "app-signin" with:
  | identifier | password | label |*
  | .ZPA       | Aa1      | Pixel |
  And reply "token" is kept as var "token"
  And cgpay posts "app-revoke" with:
  | token  |*
  | %token |
  When cgpay posts "app-device" with:
  | token  |*
  | %token |
  Then we reply "unauth" with JSON values:
  | error     |*1
  | bad-token |
  And count "app_tokens" is 1

Scenario: Signing out everywhere ends every device and its trust
  Given next random code is "12345678"
  And cgpay posts "app-signin" with:
  | identifier | password | label | deviceKey |*
  | .ZPB       | Bb2      | Pixel | key2      |
  And reply "mfa" is kept as var "mfa"
  And cgpay posts "app-mfa" with:
  | mfa  | code     | trust |*
  | %mfa | 12345678 | 1     |
  And reply "token" is kept as var "token"
  And cgpay posts "app-revoke" with:
  | token  | everywhere |*
  | %token | 1          |
  When cgpay posts "app-signin" with:
  | identifier | password | label | deviceKey |*
  | .ZPB       | Bb2      | Pixel | key2      |
  Then we reply "ok" with JSON values:
  | mfa |*1
  | ?   |

# Rule: A token opens a live page once, through a one-time code.

Scenario: A one-time code opens a live page once
  Given cgpay posts "app-signin" with:
  | identifier | password | label |*
  | .ZPA       | Aa1      | Pixel |
  And reply "token" is kept as var "token"
  And cgpay posts "app-enter" with:
  | token  | to      |*
  | %token | history |
  And reply "code" is kept as var "code"
  When cgpay posts "app-redeem" with:
  | code  |*
  | %code |
  Then we reply "ok" with JSON values:
  | uid | to      |*1
  | ?   | history |
  
Scenario: A one-time code works only once
  Given cgpay posts "app-signin" with:
  | identifier | password | label |*
  | .ZPA       | Aa1      | Pixel |
  And reply "token" is kept as var "token"
  And cgpay posts "app-enter" with:
  | token  |*
  | %token |
  And reply "code" is kept as var "code"
  And cgpay posts "app-redeem" with:
  | code  |*
  | %code |
  When cgpay posts "app-redeem" with:
  | code  |*
  | %code |
  Then we reply "unauth" with JSON values:
  | error    |*1
  | bad-code |

Scenario: A page on another site cannot be asked for
  Given cgpay posts "app-signin" with:
  | identifier | password | label |*
  | .ZPA       | Aa1      | Pixel |
  And reply "token" is kept as var "token"
  And cgpay posts "app-enter" with:
  | token  | to                     |*
  | %token | https://evil.example/x |
  And reply "code" is kept as var "code"
  When cgpay posts "app-redeem" with:
  | code  |*
  | %code |
  Then we reply "ok" with JSON values:
  | to        |*1
  | dashboard |
