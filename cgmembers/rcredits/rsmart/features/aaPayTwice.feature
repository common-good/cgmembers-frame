Feature: Transact Pay Twice

#This is conceptually part of the Transact feature, but needs to come early in the set, in order to complete successfully
#(because otherwise the creation times of transactions is too far in the past when calculated by the test compiler)

Setup:
  Given members:
  | uid  | fullName   | email | cc  | cc2  | floor | flags             | helper |*
  | .ZZA | Abe One    | a@    | ccA | ccA2 |  -250 | ok,confirmed,debt | 0      |
  | .ZZB | Bea Two    | b@    | ccB | ccB2 |  -250 | ok,confirmed,debt | 0      |
  | .ZZC | Corner Pub | c@    | ccC |      |     0 | ok,co,confirmed   | .ZZA   |
  | .ZZD | Dee Four   | d@    | ccD | ccD2 |     0 | ok,confirmed      | 0      |
  | .ZZE | Eve Five   | e@    | ccE | ccE2 |  -250 | ok,secret,roundup,debt | .ZZD   |
  | .ZZF | Far Co     | f@    | ccF |      |     0 | ok,co,confirmed   | 0      |
  And these "r_boxes":
  | uid  | code |*
  | .ZZC | devC |
  And selling:
  | uid  | selling         |*
  | .ZZC | this,that,other |
  And company flags:
  | uid  | coFlags      |*
  | .ZZC | refund,r4usd |
  And these "u_relations":
  | main | agent | num | permission |*
  | .ZZC | .ZZA  |   1 | buy        |
  | .ZZC | .ZZB  |   2 | sell       |
  | .ZZC | .ZZD  |   3 | read       |
  | .ZZF | .ZZE  |   1 | sell       |
  And these "txs": 
  | xid | created   | amount | payer | payee | purpose |*
  | 3   | %today-6m |    250 | ctty | .ZZC | growth  |
  | 5   | %today-6m |    250 | ctty | .ZZF | stuff   |
  Then balances:
  | uid  | balance |*
  | ctty |    -500 |
  | .ZZA |       0 |
  | .ZZB |       0 |
  | .ZZC |     250 |
  | .ZZE |       0 |
  | .ZZF |     250 |

Scenario: Seller tries to charge the customer twice
  Given agent "C:A" asks device "devC" to charge ".ZZB,ccB" $100 for "goods": "food" at "%now-1min"
  When agent "C:A" asks device "devC" to charge ".ZZB,ccB" $100 for "goods": "food" at %now
  Then we return error "duplicate transaction" with subs:
  | op      |*
  | charged |
