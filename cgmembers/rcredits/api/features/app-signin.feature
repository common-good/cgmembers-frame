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
  | .ZPC | Coco Co  | +20003 | c@    | ccC | ok,co  |      | Cton | CA    |
  | .ZPE | Eli Five | +20005 | e@    | ccE | ok,admin | Ee5 | Eton | NY    |
  | .ZPF | Flo Six  | +20006 | f@    | ccF | ok     | Ff6  | Fton | FL    |
  And these "u_relations":
  | main | agent | num | permission |*
  | .ZPC | .ZPA  |   1 | manage     |
  And member ".ZPA" has "card" steps done: "all"
  And member ".ZPB" has "card" steps done: "all"

# Rule: The right password signs the app in with a token, and only the token's hash is kept.

Scenario: A member signs in to the app
  When cgpay posts "signin" with:
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
  When cgpay posts "signin" with:
  | identifier | password | label |*
  | a@         | Aa1      | Pixel |
  Then we reply "ok" with JSON values:
  | token |*1
  | ?     |

Scenario: A wrong password gets no token
  When cgpay posts "signin" with:
  | identifier | password | label |*
  | .ZPA       | wrong    | Pixel |
  Then we reply "unauth" with JSON values:
  | error     |*1
  | bad-login |
  And count "app_tokens" is 0

Scenario: An unknown account gets no token
  When cgpay posts "signin" with:
  | identifier | password | label |*
  | .ZPZ       | Aa1      | Pixel |
  Then we reply "unauth" with JSON values:
  | error     |*1
  | bad-login |
  And count "app_tokens" is 0

Scenario: A caller without the shared secret is refused
  When a stranger posts "signin" with:
  | identifier | password | label |*
  | .ZPA       | Aa1      | Pixel |
  Then we reply "unauth" with: "?"
  And count "app_tokens" is 0

# Rule: A member with multi-factor authentication must also give the passcode we email.

Scenario: A member with MFA is asked for the passcode
  Given next random code is "12345678"
  When cgpay posts "signin" with:
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
  And cgpay posts "signin" with:
  | identifier | password | label | deviceKey |*
  | .ZPB       | Bb2      | Pixel | key2      |
  And reply "mfa" is kept as var "mfa"
  When cgpay posts "mfa" with:
  | mfa  | code     | trust |*
  | %mfa | 12345678 | 0     |
  Then we reply "ok" with JSON values:
  | token |*1
  | ?     |
  And these "app_tokens":
  | uid  | label | trusted |*
  | .ZPB | Pixel | 0       |

Scenario: A passcode handle signs the app in only once
  Given next random code is "12345678"
  And cgpay posts "signin" with:
  | identifier | password | label | deviceKey |*
  | .ZPB       | Bb2      | Pixel | key2      |
  And reply "mfa" is kept as var "mfa"
  And cgpay posts "mfa" with:
  | mfa  | code     | trust |*
  | %mfa | 12345678 | 0     |
  When cgpay posts "mfa" with:
  | mfa  | code     | trust |*
  | %mfa | 12345678 | 0     |
  Then we reply "unauth" with JSON values:
  | error      |*1
  | bad-handle |

Scenario: A wrong passcode gets no token
  Given next random code is "12345678"
  And cgpay posts "signin" with:
  | identifier | password | label | deviceKey |*
  | .ZPB       | Bb2      | Pixel | key2      |
  And reply "mfa" is kept as var "mfa"
  When cgpay posts "mfa" with:
  | mfa  | code     | trust |*
  | %mfa | 99999999 | 0     |
  Then we reply "unauth" with JSON values:
  | error    |*1
  | bad-code |
  And count "app_tokens" is 0

# Rule: A device the member trusts skips the passcode next time, even after signing out.

Scenario: A trusted device signs in without a passcode
  Given next random code is "12345678"
  And cgpay posts "signin" with:
  | identifier | password | label | deviceKey |*
  | .ZPB       | Bb2      | Pixel | key2      |
  And reply "mfa" is kept as var "mfa"
  And cgpay posts "mfa" with:
  | mfa  | code     | trust |*
  | %mfa | 12345678 | 1     |
  And reply "token" is kept as var "token"
  And cgpay posts "revoke" with:
  | token  |*
  | %token |
  When cgpay posts "signin" with:
  | identifier | password | label | deviceKey |*
  | .ZPB       | Bb2      | Pixel | key2      |
  Then we reply "ok" with JSON values:
  | token |*1
  | ?     |
  And count "app_tokens" is 2

Scenario: Another device is not trusted
  Given next random code is "12345678"
  And cgpay posts "signin" with:
  | identifier | password | label | deviceKey |*
  | .ZPB       | Bb2      | Pixel | key2      |
  And reply "mfa" is kept as var "mfa"
  And cgpay posts "mfa" with:
  | mfa  | code     | trust |*
  | %mfa | 12345678 | 1     |
  When cgpay posts "signin" with:
  | identifier | password | label | deviceKey |*
  | .ZPB       | Bb2      | Tablet | key3  |
  Then we reply "ok" with JSON values:
  | mfa |*1
  | ?   |

# Rule: A token gives the device credentials until it is revoked.

Scenario: The app asks for its device credentials
  Given cgpay posts "signin" with:
  | identifier | password | label |*
  | .ZPA       | Aa1      | Pixel |
  And reply "token" is kept as var "token"
  When cgpay posts "device" with:
  | token  |*
  | %token |
  Then we reply "ok" with JSON values:
  | uid |*1
  | ?   |

Scenario: A made-up token is refused
  When cgpay posts "device" with:
  | token |*
  | abc   |
  Then we reply "unauth" with JSON values:
  | error     |*1
  | bad-token |

Scenario: A signed-out token no longer works
  Given cgpay posts "signin" with:
  | identifier | password | label |*
  | .ZPA       | Aa1      | Pixel |
  And reply "token" is kept as var "token"
  And cgpay posts "revoke" with:
  | token  |*
  | %token |
  When cgpay posts "device" with:
  | token  |*
  | %token |
  Then we reply "unauth" with JSON values:
  | error     |*1
  | bad-token |
  And count "app_tokens" is 1

Scenario: Signing out everywhere ends every device and its trust
  Given next random code is "12345678"
  And cgpay posts "signin" with:
  | identifier | password | label | deviceKey |*
  | .ZPB       | Bb2      | Pixel | key2      |
  And reply "mfa" is kept as var "mfa"
  And cgpay posts "mfa" with:
  | mfa  | code     | trust |*
  | %mfa | 12345678 | 1     |
  And reply "token" is kept as var "token"
  And cgpay posts "revoke" with:
  | token  | everywhere |*
  | %token | 1          |
  When cgpay posts "signin" with:
  | identifier | password | label | deviceKey |*
  | .ZPB       | Bb2      | Pixel | key2      |
  Then we reply "ok" with JSON values:
  | mfa |*1
  | ?   |

Scenario: A new password signs the app out on every device and ends its trust
  Given next random code is "12345678"
  And cgpay posts "signin" with:
  | identifier | password | label | deviceKey |*
  | .ZPB       | Bb2      | Pixel | key2      |
  And reply "mfa" is kept as var "mfa"
  And cgpay posts "mfa" with:
  | mfa  | code     | trust |*
  | %mfa | 12345678 | 1     |
  And reply "token" is kept as var "token"
  And members have:
  | uid  | pass   |*
  | .ZPB | Bb2new |
  When cgpay posts "device" with:
  | token  |*
  | %token |
  Then we reply "unauth" with JSON values:
  | error     |*1
  | bad-token |
  And these "app_tokens":
  | uid  | trusted |*
  | .ZPB | 0       |

# Rule: A token opens a live page once, through a one-time code.

Scenario: A one-time code opens a live page once
  Given cgpay posts "signin" with:
  | identifier | password | label |*
  | .ZPA       | Aa1      | Pixel |
  And reply "token" is kept as var "token"
  And cgpay posts "enter" with:
  | token  | to      |*
  | %token | history |
  And reply "code" is kept as var "code"
  When cgpay posts "redeem" with:
  | code  |*
  | %code |
  Then we reply "ok" with JSON values:
  | uid | to      |*1
  | ?   | history |
  
Scenario: A one-time code works only once
  Given cgpay posts "signin" with:
  | identifier | password | label |*
  | .ZPA       | Aa1      | Pixel |
  And reply "token" is kept as var "token"
  And cgpay posts "enter" with:
  | token  |*
  | %token |
  And reply "code" is kept as var "code"
  And cgpay posts "redeem" with:
  | code  |*
  | %code |
  When cgpay posts "redeem" with:
  | code  |*
  | %code |
  Then we reply "unauth" with JSON values:
  | error    |*1
  | bad-code |

Scenario: A page on another site cannot be asked for
  Given cgpay posts "signin" with:
  | identifier | password | label |*
  | .ZPA       | Aa1      | Pixel |
  And reply "token" is kept as var "token"
  And cgpay posts "enter" with:
  | token  | to                     |*
  | %token | https://evil.example/x |
  And reply "code" is kept as var "code"
  When cgpay posts "redeem" with:
  | code  |*
  | %code |
  Then we reply "ok" with JSON values:
  | to        |*1
  | dashboard |

Scenario: A protocol-relative page cannot be asked for either
  Given cgpay posts "signin" with:
  | identifier | password | label |*
  | .ZPA       | Aa1      | Pixel |
  And reply "token" is kept as var "token"
  And cgpay posts "enter" with:
  | token  | to               |*
  | %token | //evil.example/x |
  And reply "code" is kept as var "code"
  When cgpay posts "redeem" with:
  | code  |*
  | %code |
  Then we reply "ok" with JSON values:
  | to        |*1
  | dashboard |

# Rule: The app gets every account the member may act for, and opens the live site acting for the one chosen.

Scenario: Signing in lists the accounts the member may act for
  When cgpay posts "signin" with:
  | identifier | password | label |*
  | .ZPA       | Aa1      | Pixel |
  Then reply list "accounts" includes:
  | uid  | name    | isCo  | device |*
  | .ZPA | Abe One | false | ?      |
  | .ZPC | Coco Co | true  | %NUL   |

Scenario: The app opens the live site acting for another account
  Given cgpay posts "signin" with:
  | identifier | password | label |*
  | .ZPA       | Aa1      | Pixel |
  And reply "token" is kept as var "token"
  And cgpay posts "enter" with:
  | account | token  |*
  | .ZPC    | %token |
  And reply "code" is kept as var "code"
  When cgpay posts "redeem" with:
  | code  |*
  | %code |
  Then we reply "ok" with JSON values:
  | to        |*1
  | dashboard |
  And these "app_codes":
  | uid  | account | dest      |*
  | .ZPA | .ZPC    | dashboard |

Scenario: The app cannot open the live site for an account the member does not manage
  Given cgpay posts "signin" with:
  | identifier | password | label |*
  | .ZPF       | Ff6      | Pixel |
  And reply "token" is kept as var "token"
  When cgpay posts "enter" with:
  | account | token  |*
  | .ZPC    | %token |
  Then we reply "forbidden" with JSON values:
  | error       |*1
  | not-allowed |

# Rule: An admin may type an account to act for; others only choose from their list.

Scenario: An admin types an account
  Given cgpay posts "signin" with:
  | identifier | password | label |*
  | .ZPE       | Ee5      | Pixel |
  And reply "token" is kept as var "token"
  When cgpay posts "account" with:
  | token  | who  |*
  | %token | .ZPB |
  Then reply list "account" includes:
  | uid  | name    |*
  | .ZPB | Bea Two |

Scenario: A member who is not an admin cannot type an account outside their list
  Given cgpay posts "signin" with:
  | identifier | password | label |*
  | .ZPA       | Aa1      | Pixel |
  And reply "token" is kept as var "token"
  When cgpay posts "account" with:
  | token  | who  |*
  | %token | .ZPB |
  Then we reply "forbidden" with JSON values:
  | error       |*1
  | not-allowed |

# Rule: The app can ask for the password again before changing account or settings.

Scenario: The right password unlocks
  Given cgpay posts "signin" with:
  | identifier | password | label |*
  | .ZPA       | Aa1      | Pixel |
  And reply "token" is kept as var "token"
  When cgpay posts "unlock" with:
  | token  | password |*
  | %token | Aa1      |
  Then we reply "ok" with JSON values:
  | ok |*1
  | 1  |

Scenario: A wrong password does not unlock
  Given cgpay posts "signin" with:
  | identifier | password | label |*
  | .ZPA       | Aa1      | Pixel |
  And reply "token" is kept as var "token"
  When cgpay posts "unlock" with:
  | token  | password |*
  | %token | wrong    |
  Then we reply "unauth" with JSON values:
  | error     |*1
  | bad-login |
