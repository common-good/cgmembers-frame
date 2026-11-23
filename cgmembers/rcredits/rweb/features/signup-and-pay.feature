Feature: A user signs up for Common Good in order to pay
AS a newbie
I WANT to open a Common Good account
SO I can pay someone

Setup:
  Given members:
  | uid  | fullName | pass | email | flags                    | zip   | floor | emailCode |*
  | .ZZA | Abe One  | a1   | a@    | member,ok,confirmed,debt | 01001 |  -100 | Aa1       |
  | .ZZB | Bea Two  | b1   | b@    | member,ok,confirmed,debt | 01001 |  -100 | Bb2       |
  | .ZZC | Our Pub  | c1   | c@    | member,ok,co,confirmed   | 01003 |     0 | Cc3       |
  And member is logged out

Scenario: A non-member clicks a CGPay button
  Given button code "buttonCode" for:
  | account | secret | item | amount |*
  | .ZZC    | Cc3    | food | 23.50  |
  When member "?" visits page "pay/code=%buttonCode"
  Then we show "Pay Our Pub" with:
  | Pay          | 23.50 |
  | For          | food  |
  | Account ID   |       |
  | Password     |       |
  | Sign up here |       |
  | Pay          |       |

# The old "newbie supplies email to continue" scenario is gone: signing in or up from this page now
# works like the regular sign-in and sign-up pages (see signin.feature and signup.feature).
