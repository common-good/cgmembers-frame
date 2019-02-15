Feature: Gift
AS a participating business
I WANT to issue gift coupons and discount coupons
SO I can reward my employees and attract customers
AS a member
I WANT to redeem a coupon or issue gift coupons
SO I can pay less for stuff or treat a friend.

Setup:
  Given members:
  | uid  | fullName   | email | cc  | cc2  | floor | flags             |*
  | .ZZA | Abe One    | a@    | ccA | ccA2 |  -250 | ok,confirmed,debt |
  | .ZZB | Bea Two    | b@    | ccB | ccB2 |  -250 | ok,confirmed,debt |
  | .ZZC | Corner Pub | c@    | ccC |      |     0 | ok,co,confirmed   |
  And devices:
  | uid  | code |*
  | .ZZC | devC |
  And selling:
  | uid  | selling         |*
  | .ZZC | this,that,other |
  And company flags:
  | uid  | coFlags      |*
  | .ZZC | refund,r4usd |
  And relations:
  | reid | main | agent | num | permission |*
  | .ZZA | .ZZC | .ZZA  |   1 | scan       |
  
Scenario: A member redeems a gift coupon
  Given  coupons:
  | coupid | fromId | amount | minimum | ulimit | flags | start  | end       |*
  |      1 |   .ZZC |     10 |       0 |      1 |     0 | %today | %today+7d |
  When agent "C:A" asks device "devC" to charge ".ZZB,ccB" $100 for "goods": "food" at %today
  Then transaction headers: 
  | xid | goods | initiator | initiatorAgent | created |*
  | 1   |   0   | .ZZC      | .ZZC           | %today  |
  And transaction entries:
  | xid | amount |  uid | description             | relType | related |*
  | 1   |    -90 | .ZZB | food                    |         |         |
  | 1   |    100 | .ZZC | food                    |         |         |
  | 1   |    -10 | .ZZC | discount rebate (on #1) |   D     |    1    |

  When agent "C:A" asks device "devC" to undo transaction with subs:
  | member | code | amount | goods | description | created |*
  | .ZZB   | ccB  | 100.00 |     1 | food        | %today  |
  Then transaction headers:
  | xid | initiator | initiatorAgent | created | reverses |*
  | 1   | .ZZC      | .ZZC           | %today  |          |
  | 2   | .ZZC      | .ZZC           | %today  |        1 |
  And transaction header count is 2
  And transaction entries:
  | xid | amount |  uid | description                           | relType | related |*
  | 1   |    -90 | .ZZB | food                                  |         |         |
  | 1   |    100 | .ZZC | food                                  |         |         |
  | 1   |    -10 | .ZZC | discount rebate (on #1)               |    D    |    1    |
  | 2   |     90 | .ZZB | food (reverses #1)                    |         |         |
  | 2   |   -100 | .ZZC | food (reverses #1)                    |         |         |
  | 2   |     10 | .ZZC | discount rebate (on #1) (reverses #1) |    D    |    1    |

  When agent "C:A" asks device "devC" to charge ".ZZB,ccB" $50 for "goods": "sundries" at %today
  Then transaction headers: 
  | xid | initiator | initiatorAgent | created | reverses |*
  | 3   | .ZZC      | .ZZC           | %today  |          |
  And transaction entries:
  | xid | amount |  uid | description             | relType | related |*
  | 3   |    -40 | .ZZB | sundries                |         |         |
  | 3   |     50 | .ZZC | sundries                |         |         |
  | 3   |    -10 | .ZZC | discount rebate (on #3) |    D    |    1    |
  
  When agent "C:A" asks device "devC" to charge ".ZZB,ccB" $60 for "goods": "stuff" at %today
  Then transaction headers: 
  | xid | initiator | initiatorAgent | created |*
  | 4   | .ZZC      | .ZZC           | %today  |
  And transaction entries:
  | xid | amount |  uid | description | relType | related |*
  | 4   |    -60 | .ZZB | stuff       |         |         |
  | 4   |     60 | .ZZC | stuff       |         |         |
  And transaction header count is 4
  And transaction entry count is 11
# ulimit has been reached, so no rebate