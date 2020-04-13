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
  Then balances:
  | uid  | balance |*
  | .ZZA |       0 |
  | .ZZB |       0 |
  | .ZZC |       0 |

Scenario: A member redeems a discount coupon
  Given  coupons:
  | coupid | fromId | amount | minimum | useMax | flags | start  | end       | on                |*
  |      1 |   .ZZC |     10 |       0 |      1 |     0 | %today | %today+7d | discount (rebate) |
  When agent "C:A" asks device "devC" to charge ".ZZB,ccB" $100 for "goods": "food" at %now
  Then transaction headers:
  | xid | goods | actorId | actorAgentId | flags  | channel | boxId  | risks | reversesXid | created |*
  | 1   | 0     | .ZZC    | .ZZA         | 0      | 3       | devC   |     0 |             | %today  |
  And transaction entries: 
  | xid | amount |  uid | agentUid | acctTid | description       | rule |*
  | 1   |    100 | .ZZC | .ZZA     | 1       | food              |      |
  | 1   |   -100 | .ZZB | .ZZB     | 1       | food              |      |
  | 1   |     10 | .ZZB | .ZZB     | 1       | discount (rebate) | 1    |
  | 1   |    -10 | .ZZC | .ZZC     | 1       | discount (rebate) | 1    |
  And balances:
  | uid  | balance |*
  | .ZZA |       0 |
  | .ZZB |     -90 |
  | .ZZC |      90 |
  
  When agent "C:A" asks device "devC" to undo transaction with subs:
  | member | code | amount | goods | description | created |*
  | .ZZB   | ccB  | 100.00 |     1 | food        | %today  |
  Then transaction headers:
  | xid | goods | actorId | actorAgentId | flags  | channel | boxId | risks | reversesXid | created |*
  | 2   | 0     | .ZZC    | .ZZA         | 0      | 3       | devC  |     0 | 1           | %today  |
  And transaction entries: 
  | xid | amount |  uid | agentUid | acctTid | description       | rule |*
  | 2   |   -100 | .ZZC | .ZZA     | 2       | food              |      |
  | 2   |    100 | .ZZB | .ZZB     | 2       | food              |      |
  | 2   |    -10 | .ZZB | .ZZB     | 2       | discount (rebate) | 1    |
  | 2   |     10 | .ZZC | .ZZC     | 2       | discount (rebate) | 1    |
  And balances:
  | uid  | balance |*
  | .ZZA |       0 |
  | .ZZB |       0 |
  | .ZZC |       0 |

  When agent "C:A" asks device "devC" to charge ".ZZB,ccB" $50 for "goods": "sundries" at %today
  Then transaction headers:
  | xid | goods | actorId | actorAgentId | flags  | channel | boxId | risks | reversesXid | created |*
  | 3   | 0     | .ZZC    | .ZZA         | 0      | 3       | devC  |     0 |             | %today  |
  And transaction entries: 
  | xid | amount |  uid | agentUid | acctTid | description       | rule |*
  | 3   |     50 | .ZZC | .ZZA     | 3       | sundries          |      |
  | 3   |    -50 | .ZZB | .ZZB     | 3       | sundries          |      |
  | 3   |    -10 | .ZZC | .ZZC     | 3       | discount (rebate) | 1    |
  | 3   |     10 | .ZZB | .ZZB     | 3       | discount (rebate) | 1    |
  And balances:
  | uid  | balance |*
  | .ZZA |       0 |
  | .ZZB |     -40 |
  | .ZZC |      40 |

  When agent "C:A" asks device "devC" to charge ".ZZB,ccB" $60 for "goods": "stuff" at %today
  Then transaction headers:
  | xid | goods | actorId | actorAgentId | flags  | channel | boxId  | risks | reversesXid | created |*
  | 4   | 0     | .ZZC    | .ZZA         | 0      | 3       | devC   |     0 |             | %today  |
  And transaction entries: 
  | xid | amount |  uid | agentUid | acctTid | description | relType | relatedId |*
  | 4   |     60 | .ZZC | .ZZA     | 4       | stuff       |         |         |
  | 4   |    -60 | .ZZB | .ZZB     | 4       | stuff       |         |         |
  And transaction header count is 4
  And balances:
  | uid  | balance |*
  | .ZZA |       0 |
  | .ZZB |    -100 |
  | .ZZC |     100 |
  
# useMax has been reached, so no rebate
