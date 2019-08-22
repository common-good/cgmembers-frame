Feature: Gift
AS a participating business
I WANT to issue gift coupons and discount coupons
SO I can reward my employees and attract customers
AS a member
I WANT to redeem a coupon or issue gift coupons
SO I can pay less for stuff or treat a friend.

Setup:
  Given members:
  | uid  | fullName   | floor | flags             |*
  | .ZZA | Abe One    |  -250 | ok,confirmed,debt |
  | .ZZB | Bea Two    |  -250 | ok,confirmed,debt |
  | .ZZC | Corner Pub |  -250 | ok,confirmed,co   |  

Scenario: A member company creates a gift coupon
  Given members have:
  | uid  | giftCoupons |*
  | .ZZC |           8 |
  When member ".ZZC" completes form "community/coupons/type=gift" with values:
  | type | amount | count |*
  | gift |     10 |    20 |
  Then coupons:
  | coupid | fromId | amount | ulimit | flags | start | end |*
  |      1 |   .ZZC |     10 |      1 | gift  |     8 |  28 |
#  And member ".ZZC" visits page "community/coupons/print/type=gift&amount=10&ulimit=1&count=20", which results in:
#  When member ".ZZC" visits page "community/coupons/print/type=gift&amount=10&count=20"
  And members have:
  | uid  | giftCoupons |*
  | .ZZC |          28 |
  When member ".ZZC" visits page "community/coupons/list"
  Then we show "Your Discounts and Gift Certificates" with:
  | Type | Amount | On | Starting | Ending | Min Purchase | Max Uses |~Action  |
  | gift | $10    |    |        8 |     28 |           $0 |        1 | reprint |
  
Scenario: A member redeems a gift coupon
  Given members have:
  | uid  | giftCoupons | created |*
  | .ZZC |           8 | 0039200 |
# created determines 3-letter lowSecurity code (7AA), which is used in coupon code
  And coupons:
  | coupid | fromId | amount | ulimit | flags | start | end | sponsor |*
  |      1 |   .ZZC |     10 |      1 | gift  |     8 |  28 | .ZZC    |
  When member ".ZZA" completes form "community/coupons/type=gift" with values:
  | type   | code          |*
  | redeem | DD7K CLJW EAI |
  Then balances:
  | uid  | balance |*
  | .ZZA |      10 |
  | .ZZC |     -10 |
  And members have:
  | uid  | giftPot |*
  | .ZZA |      10 |
  When member ".ZZB" completes form "community/coupons/type=gift" with values:
  | type   | code          |*
  | redeem | DD7K CLJW EAI |
  Then we say "error": "already redeemed"

Scenario: A member company creates a dollar amount discount coupon
  When member ".ZZC" visits page "community/coupons/type=discount"
  Then we show "Create a Discount" with:
 | Discount: |||
 | Minimum: |||
 | Valid from: |||
 | Valid until: |||
 | Limit:  |||
 | For only: |||
 | Automatic? | No | Yes |
 
  
  When member ".ZZC" completes form "community/coupons/type=discount" with values:
  | type     | amount | minimum | start | end     | ulimit | automatic |*
  | discount |     12 |      20 | %mdY  | %mdY+9d |      1 |         1 |
  Then coupons:
  | coupid | amount | fromId | minimum | ulimit | flags | start     | end                | sponsor |*
  |      1 |     12 |   .ZZC |      20 |      1 |       | %daystart | %(%daystart+10d-1) | .ZZC    |
  When member ".ZZC" visits page "community/coupons/list"
  Then we show "Your Discounts and Gift Certificates" with:
  | Type     | Amount | On                              | Starting | Ending  | Min Purchase | Max Uses |~Action  |
  | discount | $12    | on your purchase of $20 or more | %mdY     | %mdY+9d |          $20 |        1 | reprint |
  When member ".ZZA" visits page "community/coupons/list/ALL"
  Then we show "Automatic Discounts in Your Region" with:
  | Company    | Discount | On                              | Ending  | For    | Uses Left |
  | Corner Pub | $12      | on your purchase of $20 or more | %mdY+9d | anyone | 1 of 1    |
  
Scenario: A member redeems a dollar amount discount coupon
  Given coupons:
  | coupid | amount | fromId | minimum | ulimit | flags | start     | end                | sponsor | on |*
  |      1 |     12 |   .ZZC |      20 |      1 |       | %daystart | %(%daystart+10d-1) |.ZZC| on your purchase of $20 or more |
  When member ".ZZA" confirms form "pay" with values:
  | op  | who        | amount | purpose |*
  | pay | Corner Pub | 100    | fun     |
  Then we say "status": "report tx" with subs:
  | did    | otherName  | amount |*
  | paid   | Corner Pub | $100   |
  And these "txs":
  | eid | xid | type   | created | amount | from  | to   | purpose           | taking |*
  |   1 |   1 | prime  | %today  |    100 | .ZZA  | .ZZC | fun               |      0 |
  |   2 |   1 | rebate | %today  |     12 | .ZZC  | .ZZA | discount (rebate) |      1 |
  And balances:
  | uid  | balance |*
  | .ZZA |     -88 |
  | .ZZB |       0 |
  | .ZZC |      88 |
  When member ".ZZA" visits page "community/coupons/list/ALL"
  Then we show "Automatic Discounts in Your Region" with:
  | Company    | Discount | On                              | Ending  | For    | Uses Left |
  | Corner Pub | $12      | on your purchase of $20 or more | %mdY+9d | anyone | 0 of 1    |
  
  When member ".ZZA" confirms form "pay" with values:
  | op  | who        | amount | purpose |*
  | pay | Corner Pub | 40     | fun     |
  Then balances:
  | uid  | balance |*
  | .ZZA |    -128 |
  | .ZZC |     128 |

Scenario: A member redeems a percentage discount coupon
  Given coupons:
  | coupid | fromId | amount | minimum | ulimit | flags | start     | end                | sponsor | on                        |*
  |      7 |   .ZZC |    -12 |      20 |      2 |       | %daystart | %(%daystart+10d-1) |.ZZC| on your purchase of $20 or more |
  When member ".ZZA" confirms form "pay" with values:
  | op  | who        | amount | purpose |*
  | pay | Corner Pub | 50     | fun     |
  Then balances:
  | uid  | balance |*
  | .ZZA |     -44 |
  | .ZZB |       0 |
  | .ZZC |      44 |
  When member ".ZZA" visits page "community/coupons/list/ALL"
  Then we show "Automatic Discounts in Your Region" with:
  | Company    | Discount | On                              | Ending  | For    | Uses Left |
  | Corner Pub | 12.0%    | on your purchase of $20 or more | %mdY+9d | anyone | 1 of 2    |

  When member ".ZZA" confirms form "pay" with values:
  | op  | who        | amount | purpose |*
  | pay | Corner Pub | 50     | fun     |
  Then balances:
  | uid  | balance |*
  | .ZZA |     -88 |
  | .ZZB |       0 |
  | .ZZC |      88 |
  When member ".ZZA" confirms form "pay" with values:
  | op  | who        | amount | purpose |*
  | pay | Corner Pub | 50     | fun     |
  Then balances:
  | uid  | balance |*
  | .ZZA |    -138 |
  | .ZZB |       0 |
  | .ZZC |     138 |
  When member ".ZZB" confirms form "pay" with values:
  | op  | who        | amount | purpose |*
  | pay | Corner Pub | 50     | fun     |
  Then balances:
  | uid  | balance |*
  | .ZZA |    -138 |
  | .ZZB |     -44 |
  | .ZZC |     182 |

Scenario: A member company creates a restricted dollar amount discount coupon
  When member ".ZZC" completes form "community/coupons/type=discount" with values:
  | type     | amount | minimum | start | end     | ulimit | automatic | forOnly |*
  | discount |     12 |      20 | %mdY  | %mdY+9d |      1 |         1 | NEWZZA  |
  Then coupons:
  | coupid | amount | fromId | minimum | ulimit | flags | start     | end                |*
  |      1 |     12 |   .ZZC |      20 |      1 | some  | %daystart | %(%daystart+10d-1) |
  And these "coupated":
  | id | uid  | coupid | uses | when |*
  |  1 | .ZZA |      1 |    0 |    0 |
  When member ".ZZC" visits page "community/coupons/list"
  Then we show "Your Discounts and Gift Certificates" with:
  | Type     | Amount | On                              | Starting | Ending  | Min Purchase | Max Uses |~Action  |
  | discount | $12    | on your purchase of $20 or more | %mdY     | %mdY+9d |          $20 |        1 | reprint |
  | for only: NEWZZA ||||||||
  When member ".ZZA" visits page "community/coupons/list/ALL"
  Then we show "Automatic Discounts in Your Region" with:
  | Company    | Discount | On                              | Ending  | For    | Uses Left |
  | Corner Pub | $12      | on your purchase of $20 or more | %mdY+9d | you+   | 1         |
  
Scenario: A member redeems a restricted discount coupon
  Given coupons:
  | coupid | amount | minimum | fromId | ulimit | flags | start     | end                | sponsor | on           |*
  |      1 |     20 |       0 |   .ZZC |      0 | some  | %daystart | %(%daystart+10d-1) | .ZZC    | any purchase |
  And these "coupated":
  | id | uid  | coupid | when |*
  |  1 | .ZZA |      1 |    0 |
  When member ".ZZA" visits page "community/coupons/list/ALL"
  Then we show "Automatic Discounts in Your Region" with:
  | Company    | Discount | On           | Ending  | For    | Uses Left |
  | Corner Pub | $20      | any purchase | %mdY+9d | you+   | $20       |

  When member ".ZZA" confirms form "pay" with values:
  | op  | who        | amount | purpose |*
  | pay | Corner Pub | 6      | fun     |
  Then balances:
  | uid  | balance |*
  | .ZZA |       0 |
  | .ZZB |       0 |
  | .ZZC |       0 |
  And these "coupated":
  | id | uid  | coupid | when   |*
  |  1 | .ZZA |      1 | %today |
  When member ".ZZA" visits page "community/coupons/list/ALL"
  Then we show "Automatic Discounts in Your Region" with:
  | Company    | Discount | On           | Ending  | For    | Uses Left |
  | Corner Pub | $20      | any purchase | %mdY+9d | you+   | $14       |
  
  When member ".ZZB" confirms form "pay" with values:
  | op  | who        | amount | purpose |*
  | pay | Corner Pub | 50     | fun     |
  Then balances:
  | uid  | balance |*
  | .ZZA |       0 |
  | .ZZB |     -50 |
  | .ZZC |      50 |
  
Scenario: A member redeems a discount coupon in dribs and drabs
  Given coupons:
  | coupid | amount | minimum | fromId | ulimit | flags | start     | end                | sponsor |*
  |      1 |     20 |       0 |   .ZZC |      0 |       | %daystart | %(%daystart+10d-1) | .ZZC    |
  When member ".ZZA" confirms form "pay" with values:
  | op  | who        | amount | purpose |*
  | pay | Corner Pub |     12 | fun     |
  Then we say "status": "You paid Corner Pub $12."
  And balances:
  | uid  | balance |*
  | .ZZA |       0 |
  | .ZZB |       0 |
  | .ZZC |       0 |
  When member ".ZZA" confirms form "pay" with values:
  | op  | who        | amount | purpose |*
  | pay | Corner Pub |     12 | fun     |
  Then we say "status": "You paid Corner Pub $12."
  And balances:
  | uid  | balance |*
  | .ZZA |      -4 |
  | .ZZB |       0 |
  | .ZZC |       4 |
  When member ".ZZA" confirms form "pay" with values:
  | op  | who        | amount | purpose |*
  | pay | Corner Pub |     12 | fun     |
  Then we say "status": "You paid Corner Pub $12."
  And balances:
  | uid  | balance |*
  | .ZZA |     -16 |
  | .ZZB |       0 |
  | .ZZC |      16 |  

Scenario: A member with nothing redeems a zero minimum discount coupon
  Given members have:
  | uid  | flags        |*
  | .ZZA | ok,confirmed |
  And coupons:
  | coupid | amount | minimum | fromId | ulimit | flags | start     | end                | sponsor |*
  |      1 |     20 |       0 |   .ZZC |      0 |       | %daystart | %(%daystart+10d-1) | .ZZC    |
  When member ".ZZA" confirms form "pay" with values:
  | op  | who        | amount | purpose |*
  | pay | Corner Pub |     12 | fun     |
  Then we say "status": "You paid Corner Pub $12."
  And balances:
  | uid  | balance |*
  | .ZZA |       0 |
  | .ZZB |       0 |
  | .ZZC |       0 |

Scenario: A member redeems a discount coupon sponsored by a third party
  Given coupons:
  | coupid | amount | minimum | fromId | ulimit | flags | start     | end                | sponsor |*
  |      1 |     12 |       0 |   .ZZC |      0 |       | %daystart | %(%daystart+10d-1) | .ZZB    |
  When member ".ZZA" confirms form "pay" with values:
  | op  | who        | amount | purpose |*
  | pay | Corner Pub |     20 | fun     |
  Then we say "status": "You paid Corner Pub $20."
  And balances:
  | uid  | balance |*
  | .ZZA |      -8 |
  | .ZZB |     -12 |
  | .ZZC |      20 |
