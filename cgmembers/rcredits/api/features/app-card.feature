Feature: App Card Sign-in
AS a member using the CGPay store app
I WANT to switch the app to demo and back by scanning a card
SO I can show the app without real money, and get back to my real account easily
# The store app calls /cgpay/card (server-to-server) when a scanned card does not match the server it is on:
# a test card (RC4.ME) on any server but demo, or a real card (RC2.ME) on demo.

Setup:
  Given members:
  | uid  | fullName | phone  | email | cc  | cc2  | flags  | pass | city | state |*
  | .ZPA | Abe One  | +20001 | a@    | ccA | ccA2 | ok     | Aa1  | Aton | AL    |
  | .ZPB | Bea Two  | +20002 | b@    | ccB | ccB2 | ok,mfa | Bb2  | Bton | MA    |
  | .ZPC | Coco Co  | +20003 | c@    | ccC |      | ok,co  |      | Cton | CA    |
  | .ZPD | Dee Four | +20004 | d@    | ccD |      |        | Dd4  | Dton | DC    |
  And these "u_relations":
  | main | agent | num | permission |*
  | .ZPC | .ZPA  |   1 | manage     |
  | .ZPC | .ZPB  |   2 | manage     |
  And member ".ZPA" has "card" steps done: "all"
  And member ".ZPB" has "card" steps done: "all"

# Rule: On demo a test card alone signs the app in, as the cardholder.

Scenario: A test card signs the app in on demo
  Given this server is demo
  And var "card" is the "test" card of ".ZPA"
  When cgpay posts "card" with:
  | qr    | label | deviceKey |*
  | %card | Pixel | key1      |
  Then we reply "ok" with JSON values:
  | token | uid  | account |*1
  | ?     | .ZPA | .ZPA    |
  And these "app_tokens":
  | uid  | label | trusted |*
  | .ZPA | Pixel | 0       |

Scenario: A company card signs in its cardholder, acting for the company
  Given this server is demo
  And var "card" is the "test" card of ".ZPC:.ZPA"
  When cgpay posts "card" with:
  | qr    | label |*
  | %card | Pixel |
  Then we reply "ok" with JSON values:
  | uid  | account |*1
  | .ZPA | .ZPC    |

Scenario: A member with MFA who skips it on demo gets no passcode
  Given this server is demo
  And var "card" is the "test" card of ".ZPB"
  When cgpay posts "card" with:
  | qr    | skipMfa | label |*
  | %card | 1       | Pixel |
  Then we reply "ok" with JSON values:
  | token | uid  |*1
  | ?     | .ZPB |
  And count "app_tokens" is 1

Scenario: A member with MFA who does not skip it on demo is asked for the passcode
  Given this server is demo
  And next random code is "12345678"
  And var "card" is the "test" card of ".ZPB"
  When cgpay posts "card" with:
  | qr    | label |*
  | %card | Pixel |
  Then we reply "ok" with JSON values:
  | mfa |*1
  | ?   |
  And we email "confirm-signin" to member "b@" with subs:
  | nonce    |*
  | 12345678 |
  And count "app_tokens" is 0

Scenario: The passcode after a company card keeps the company
  Given this server is demo
  And next random code is "12345678"
  And var "card" is the "test" card of ".ZPC:.ZPB"
  And cgpay posts "card" with:
  | qr    | label |*
  | %card | Pixel |
  And reply "mfa" is kept as var "mfa"
  When cgpay posts "mfa" with:
  | mfa  | code     | trust |*
  | %mfa | 12345678 | 0     |
  Then we reply "ok" with JSON values:
  | uid  | account |*1
  | .ZPB | .ZPC    |

# Rule: Anywhere but demo a real card needs the cardholder's password too, so a card alone never opens a real account.

Scenario: A real card without the password is refused
  Given var "card" is the "real" card of ".ZPA"
  When cgpay posts "card" with:
  | qr    | label |*
  | %card | Pixel |
  Then we reply "unauth" with JSON values:
  | error           |*1
  | password-needed |
  And count "app_tokens" is 0

Scenario: A real card with a wrong password is refused
  Given var "card" is the "real" card of ".ZPA"
  When cgpay posts "card" with:
  | qr    | password | label |*
  | %card | wrong    | Pixel |
  Then we reply "unauth" with JSON values:
  | error     |*1
  | bad-login |
  And count "app_tokens" is 0

Scenario: A real card with the password signs the app in
  Given var "card" is the "real" card of ".ZPC:.ZPA"
  When cgpay posts "card" with:
  | qr    | password | label |*
  | %card | Aa1      | Pixel |
  Then we reply "ok" with JSON values:
  | token | uid  | account |*1
  | ?     | .ZPA | .ZPC    |

Scenario: Skipping MFA is ignored anywhere but demo
  Given next random code is "12345678"
  And var "card" is the "real" card of ".ZPB"
  When cgpay posts "card" with:
  | qr    | password | skipMfa | label |*
  | %card | Bb2      | 1       | Pixel |
  Then we reply "ok" with JSON values:
  | mfa |*1
  | ?   |
  And count "app_tokens" is 0

# Rule: A card of the wrong kind for the server, or a wrong card code, gets no token.

Scenario: A test card is refused anywhere but demo
  Given var "card" is the "test" card of ".ZPA"
  When cgpay posts "card" with:
  | qr    | password | label |*
  | %card | Aa1      | Pixel |
  Then we reply "denied" with JSON values:
  | error      |*1
  | wrong-kind |

Scenario: A real card is refused on demo
  Given this server is demo
  And var "card" is the "real" card of ".ZPA"
  When cgpay posts "card" with:
  | qr    | label |*
  | %card | Pixel |
  Then we reply "denied" with JSON values:
  | error      |*1
  | wrong-kind |

Scenario: A card with a wrong card code is refused
  Given this server is demo
  When cgpay posts "card" with:
  | qr                         | label |*
  | HTTP://NEW.RC4.ME/KDCAwrong | Pixel |
  Then we reply "unauth" with JSON values:
  | error    |*1
  | bad-card |
  And count "app_tokens" is 0

# Rule: Each server's tokens carry its tag, so the app can tell them apart and no server takes another's.

Scenario: A token says which server it is for
  When cgpay posts "signin" with:
  | identifier | password | label |*
  | .ZPA       | Aa1      | Pixel |
  Then reply "token" starts with this server's tag

Scenario: A token from another server is refused
  Given member ".ZPA" has a token from server "demo" kept as var "token"
  When cgpay posts "device" with:
  | token  |*
  | %token |
  Then we reply "unauth" with JSON values:
  | error     |*1
  | bad-token |
