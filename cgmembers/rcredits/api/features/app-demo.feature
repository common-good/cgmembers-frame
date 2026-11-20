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
