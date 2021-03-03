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

Scenario: A member clicks a CGPay button
  Given a button code for:
  | account | secret | item | amount |*
  | .ZZC    | Cc3    | food | 23.50  |
  When member "?" visits page "cgpay?code=TESTCODE"
  Then we show "Pay With %PROJECT" with:
  | Pay      | 23.50 to Our Pub |
  | For      | food |
  | Email    |  |
  | Continue |  |
  
Scenario: A newbie supplies email to continue CGPay
  Given a button code for:
  | account | secret | item | amount |*
  | .ZZC    | Cc3    | food | 23     |
  And next random code is "WHATEVER"
  And next random password is "quick brown fox jumped"
  When member "?" clicks "continue" on page "cgpay?code=TESTCODE" with:
  | name |
  | d@   |
  Then we show "Submit"
  And we email "verify" to member "d@" with subs:
  | fullName | qid    | site      | code     | pass                   |*
  | Al Aargh | NEWAAA | %BASE_URL | WHATEVER | quick brown fox jumped |

  
  