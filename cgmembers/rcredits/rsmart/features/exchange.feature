Feature: Exchange
AS a company agent
I WANT to transfer rCredits to or from another member in exchange for cash
SO my company can accept cash deposits and give customers cash.

# We will eventually need variants or separate feature files for neighbor (member of different community within the region) to member, etc. and foreigner (member on a different server) to member, etc.
# cc means cardCode
# while testing, bonus is artificially set at twice the rebate amount

Setup:
  Given members:
  | uid  | fullName   | email | cc  | cc2  | floor | flags      |*
  | .ZZA | Abe One    | a@    | ccA | ccA2 |  -350 | ok,confirmed,debt    |
  | .ZZB | Bea Two    | b@    | ccB | ccB2 |  -150 | ok,confirmed,debt    |
  | .ZZC | Corner Pub | c@    | ccC |      |  -250 | ok,confirmed,co,debt |
  | .ZZD | Dee Four   | d@    | ccD | ccD2 |     0 | ok,confirmed         |
  | .ZZE | Eve Five   | e@    | ccE | ccE2 |     0 | ok,confirmed,secret  |
  | .ZZF | Far Co     | f@    | ccF |      |     0 | ok,confirmed,co      |
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
  | 4   | %today-5m |    100 | .ZZC | .ZZB | cash    |
  | 5   | %today-5m |    200 | .ZZA | .ZZC | cash    |
  | 6   | %today-4m |    250 | ctty | .ZZF | stuff   |
  Then balances:
  | uid  | balance |*
  | ctty |    -250 |
  | .ZZA |    -200 |
  | .ZZB |     100 |
  | .ZZC |     100 |
  | .ZZF |     250 |

#Variants: with/without an agent
#  | ".ZZB" asks device "devC" | ".ZZB" asks device "codeC" | ".ZZA" $ | ".ZZC" $ | # agent to member |
#  | ".ZZB" asks device "devC" | ".ZZB" asks device "codeC" | ".ZZA" $ | ".ZZC" $ | # agent to agent  |

Scenario: A cashier asks to charge someone for cash
  When agent "C:A" asks device "devC" to charge ".ZZB,ccB" $100 for "cash": "cash out" at %now
  Then we respond ok txid 7 created %now balance 0
  And with message "report tx" with subs:
  | did     | otherName | amount |*
  | charged | Bea Two   | $100   |
  And with did
  | we | did     | amount | forCash  |*
  | We | charged | $100   | for USD |
  And with undo
  | created | amount | tofrom | otherName |*
  | %dmy    | $100   | from   | Bea Two   |
  And we message "you got cash" to member ".ZZB" with subs:
  | created | otherName  | amount | payerPurpose |*
  | %now    | Corner Pub | $100   | cash out     |
  And balances:
  | uid  | balance |*
  | ctty |    -250 |
  | .ZZA |    -200 |
  | .ZZB |       0 |
  | .ZZC |     200 |
  | .ZZF |     250 |

Scenario: A cashier asks to refund someone
  When agent "C:A" asks device "devC" to charge ".ZZB,ccB" $-100 for "cash": "cash in" at %now
  Then we respond ok txid 7 created %now balance 200
  And with message "report tx" with subs:
  | did      | otherName | amount |*
  | credited | Bea Two   | $100   |
  And with did
  | we | did      | amount | forCash  |*
  | We | credited | $100   | for USD |
  And with undo
  | created | amount | tofrom | otherName |*
  | %dmy    | $100   | to     | Bea Two   |
  And we message "you gave cash" to member ".ZZB" with subs:
  | created | otherName  | amount | payeePurpose |*
  | %now    | Corner Pub | $100   | cash in      |
  And balances:
  | uid  | balance |*
  | ctty |    -250 |
  | .ZZA |    -200 |
  | .ZZB |     200 |
  | .ZZC |       0 |

Scenario: A cashier asks to charge another member, with insufficient balance
  When agent "C:A" asks device "devC" to charge ".ZZB,ccB" $300 for "cash": "cash out" at %now
  Then we return error "short from" with subs:
  | otherName | short | avail |*
  | Bea Two   | $200  |  $100 |

Scenario: A cashier asks to refund another member, with insufficient balance
  When agent "C:A" asks device "devC" to charge ".ZZB,ccB" $-400 for "cash": "cash in" at %now
  Then we return error "short to" with subs:
  | short | avail |*
  | $300  |  $100 |

Scenario: A cashier asks to pay self
  When agent "C:A" asks device "devC" to charge ".ZZC,ccC" $300 for "cash": "cash out" at %now
  Then we return error "shoulda been login"

Scenario: Device gives no member id
  When agent "C:A" asks device "devC" to charge "" $300 for "cash": "cash out" at %now
  Then we return error "missing member"
  
Scenario: Device gives bad account id
  When agent "C:A" asks device "devC" to charge "whatever,ccB" $300 for "cash": "cash out" at %now
  Then we return error "bad member"

Scenario: Device gives no amount
  When agent "C:A" asks device "devC" to charge ".ZZB,ccB" $"" for "cash": "cash out" at %now
  Then we return error "bad amount"
  
Scenario: Device gives bad amount
  When agent "C:A" asks device "devC" to charge ".ZZB,ccB" $%whatever for "cash": "cash out" at %now
  Then we return error "bad amount"
  
Scenario: Device gives too big an amount
  When agent "C:A" asks device "devC" to charge ".ZZB,ccB" $10,000,000 for "cash": "cash out" at %now
  Then we return error "amount too big" with subs:
  | max                              |*
  | %(number_format(%MAX_AMOUNT, 2)) |

Scenario: Device gives no purpose for goods and services
  When agent "C:A" asks device "devC" to charge ".ZZB,ccB" $100 for "goods": "" at %now
  Then we return error "missing description"

Scenario: Seller agent lacks permission to buy
  When agent "C:B" asks device "devC" to charge ".ZZB,ccB" $-100 for "goods": "refund" at %now
  Then we return error "no perm" with subs:
  | what    |*
  | refunds |

Scenario: Seller agent lacks permission to scan and sell
  When agent "C:D" asks device "devC" to charge ".ZZA,ccA" $100 for "cash": "cash out" at %now
  Then we return error "no perm" with subs:
  | what  |*
  | sales |
  
Scenario: Buyer agent lacks permission to buy
  When agent "C:A" asks device "devC" to charge "F:E,ccE2" $100 for "cash": "cash out" at %now
  Then we return error "other no perm" with subs:
  | otherName | what      |*
  | Eve Five  | purchases |
  
Scenario: Device sends wrong proof
  When agent "C:A" asks device "devC" to charge ".ZZB,whatever" $100 for "cash": "cash out" at %now
  Then we return error "bad proof"
  