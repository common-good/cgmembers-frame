Feature: App Demo Sign-in
AS a store reviewer or a prospective member trying the CGPay store app
I WANT to type "demo" and "pass" on its sign-in screen
SO I get a fresh demo account to try the app with, without any card or sign-up
# /cgpay/demo (server-to-server) makes the account and signs in as it. It works on the demo server only.

Setup:
  Given members:
  | uid  | fullName | phone  | email | cc  | flags | pass | city | state |*
  | .ZPA | Abe One  | +20001 | a@    | ccA | ok    | Aa1  | Aton | AL    |

# Rule: On demo each call makes a new account and signs in as it at once (no MFA).

Scenario: A visitor gets a fresh demo account
  Given this server is demo
  When cgpay posts "demo" with:
  | label | deviceKey |*
  | Pixel | key1      |
  Then we reply "ok" with JSON values:
  | token | uid |*1
  | ?     | ?   |
  And count "app_tokens" is 1

Scenario: A second visitor gets a token of their own
  Given this server is demo
  And cgpay posts "demo" with:
  | label | deviceKey |*
  | Pixel | key1      |
  When cgpay posts "demo" with:
  | label | deviceKey |*
  | Pixel | key2      |
  Then we reply "ok" with JSON values:
  | token |*1
  | ?     |
  And count "app_tokens" is 2

# Rule: Anywhere but demo there is nothing to make.

Scenario: A real server refuses
  When cgpay posts "demo" with:
  | label | deviceKey |*
  | Pixel | key1      |
  Then we reply "notfound" with JSON values:
  | error    |*1
  | not-demo |
  And count "app_tokens" is 0

Scenario: A demo account is marked for the purge
  Given this server is demo
  And cgpay posts "demo" with:
  | label | deviceKey |*
  | Pixel | key1      |
  And reply "uid" is kept as var "uid"
  Then account var "uid" is a demo account

# Rule: One address gets at most 20 demo accounts an hour, and all addresses together at most 500 a day.

Scenario: An address that made 20 demo accounts this hour gets no more
  Given this server is demo
  And this address made 20 demo accounts this hour
  When cgpay posts "demo" with:
  | label |*
  | Pixel |
  Then we reply "toomany" with JSON values:
  | error    |*1
  | flooding |
  And count "app_tokens" is 0

Scenario: After 500 demo accounts in a day, nobody gets another
  Given this server is demo
  And 500 demo accounts were made today
  When cgpay posts "demo" with:
  | label |*
  | Pixel |
  Then we reply "toomany" with JSON values:
  | error    |*1
  | flooding |

# Rule: A demo account nobody has used for 7 days is deleted, with its sign-ins.

Scenario: Unused demo accounts are purged
  Given this server is demo
  And members:
  | uid  | fullName  | flags | source         | created |*
  | .ZPD | Demo Old  | ok    | store app demo | %now-8d |
  | .ZPE | Demo Used | ok    | store app demo | %now-8d |
  | .ZPF | Demo New  | ok    | store app demo | %now-1d |
  | .ZPG | Real Old  | ok    |                | %now-8d |
  And these "app_tokens":
  | uid  | tokenHash | label | trusted | created | lastUsed | expires  |*
  | .ZPD | h1        | Pixel | 0       | %now-8d | %now-8d  | %now+80d |
  | .ZPE | h2        | Pixel | 0       | %now-8d | %now-1d  | %now+80d |
  When demo accounts are purged
  Then member ".ZPD" is gone
  And these "users":
  | uid  |*
  | .ZPE |
  | .ZPF |
  | .ZPG |
  And count "app_tokens" is 1
