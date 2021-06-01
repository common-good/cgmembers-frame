Feature: Transact
AS a company agent
I WANT to transfer rCredits to or from another member
SO my company can sell stuff, give refunds, and trade rCredits for US Dollars.

# We will eventually need variants or separate feature files for neighbor (member of different community within the region) to member, etc. and foreigner (member on a different server) to member, etc.
# cc means cardCode

Setup:
  Given members:
  | uid  | fullName   | email | cc  | cc2  | floor | flags             | helper |*
  | .ZZA | Abe One    | a@    | ccA | ccA2 |  -250 | ok,confirmed,debt | 0      |
  | .ZZB | Bea Two    | b@    | ccB | ccB2 |  -250 | ok,confirmed,debt | 0      |
  | .ZZC | Corner Pub | c@    | ccC |      |     0 | ok,co,confirmed   | .ZZA   |
  | .ZZD | Dee Four   | d@    | ccD | ccD2 |     0 | ok,confirmed      | 0      |
  | .ZZE | Eve Five   | e@    | ccE | ccE2 |  -250 | ok,secret,roundup,debt | .ZZD   |
  | .ZZF | Far Co     | f@    | ccF |      |     0 | ok,co,confirmed   | 0      |
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
  | main | agent | num | permission |*
  | .ZZC | .ZZA  |   1 | buy        |
  | .ZZC | .ZZB  |   2 | scan       |
  | .ZZC | .ZZD  |   3 | read       |
  | .ZZF | .ZZE  |   1 | sell       |
  And transactions: 
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

#Variants: with/without an agent
#  | ".ZZB" asks device "devC" | ".ZZB" asks device "codeC" | ".ZZA" $ | ".ZZC" $ | # agent to member |
#  | ".ZZB" asks device "devC" | ".ZZB" asks device "codeC" | ".ZZA" $ | ".ZZC" $ | # agent to agent  |

Scenario: A cashier asks to charge someone, paying with credit line
  When agent "C:A" asks device "devC" to charge ".ZZB,ccB" $100 for "goods": "food" at %now
  # cash exchange would be for "cash": "cash out"
  Then we respond ok txid 6 created %now balance -100 saying:
  | did     | otherName | amount | why   |*
  | charged | Bea Two   | $100   | goods |
  And with did
  | we | did     | amount | forCash |*
  | We | charged | $100   |         |
  And with undo
  | created | amount | tofrom | otherName |*
  | %dmy    | $100   | from   | Bea Two   |
  And we notice "new charge" to member ".ZZB" with subs:
  | created | fullName | otherName  | amount | payerPurpose |*
  | %now    | Bea Two  | Corner Pub | $100   | food         |
  And balances:
  | uid  | balance |*
  | ctty |    -500 |
  | .ZZA |       0 |
  | .ZZB |    -100 |
  | .ZZC |     350 |

Scenario: A cashier asks to refund someone
  When agent "C:A" asks device "devC" to charge ".ZZB,ccB" $-100 for "goods": "food" at %now
  Then we respond ok txid 6 created %now balance 100 saying:
  | did      | otherName | amount | why   |*
  | refunded | Bea Two   | $100   | goods |
  And with did
  | we | did      | amount | forCash |*
  | We | refunded | $100   |         |
  And with undo
  | created | amount | tofrom | otherName |*
  | %dmy    | $100   | to     | Bea Two   |
  And we notice "new refund" to member ".ZZB" with subs:
  | created | fullName | otherName  | amount | payerPurpose |*
  | %now    | Bea Two  | Corner Pub | $100   | food         |
  And balances:
  | uid  | balance |*
  | ctty |    -500 |
  | .ZZA |       0 |
  | .ZZB |     100 |
  | .ZZC |     150 |

Scenario: A cashier asks to charge another member, with insufficient balance
  When agent "C:A" asks device "devC" to charge ".ZZB,ccB" $300 for "goods": "food" at %now
  Then we return error "short from" with subs:
  | otherName | short | avail |*
  | Bea Two   |  $50  |  $250 |

Scenario: A cashier asks to refund another member, with insufficient balance
  When agent "C:A" asks device "devC" to charge ".ZZB,ccB" $-300 for "goods": "food" at %now
  Then we return error "short to" with subs:
  | short | avail |*
  |  $50  |  $250 |

Scenario: A cashier asks to pay self
  When agent "C:A" asks device "devC" to charge ".ZZC,ccC" $300 for "goods": "food" at %now
  Then we return error "shoulda been login"

Scenario: Device gives no member id
  When agent "C:A" asks device "devC" to charge "" $300 for "goods": "food" at %now
  Then we return error "missing member"
  
Scenario: Device gives bad account id
  When agent "C:A" asks device "devC" to charge "whatever,ccB" $300 for "goods": "food" at %now
  Then we return error "bad member"

Scenario: Device gives no amount
  When agent "C:A" asks device "devC" to charge ".ZZB,ccB" $"" for "goods": "food" at %now
  Then we return error "bad amount"
  
Scenario: Device gives bad amount
  When agent "C:A" asks device "devC" to charge ".ZZB,ccB" $%whatever for "goods": "food" at %now
  Then we return error "bad amount"
  
Scenario: Device gives too big an amount
  When agent "C:A" asks device "devC" to charge ".ZZB,ccB" $10000000 for "goods": "food" at %now
  Then we return error "amount too big" with subs:
  | max           |*
  | %MAX_AMOUNT |

Scenario: Device gives no purpose for goods and services
  When agent "C:A" asks device "devC" to charge ".ZZB,ccB" $100 for "goods": "" at %now
  Then we return error "missing description"

Scenario: Seller agent lacks permission to buy
  When agent "C:B" asks device "devC" to charge ".ZZB,ccB" $-100 for "goods": "refund" at %now
  Then we return error "no perm" with subs:
  | what    |*
  | refunds |

Scenario: Seller agent lacks permission to scan and sell
  When agent "C:D" asks device "devC" to charge ".ZZA,ccA" $100 for "goods": "food" at %now
  Then we return error "no perm" with subs:
  | what  |*
  | sales |
  
Scenario: Buyer agent lacks permission to buy
  When agent "C:A" asks device "devC" to charge "F:E,ccE2" $100 for "goods": "food" at %now
  Then we return error "other no perm" with subs:
  | otherName | what      |*
  | Eve Five  | purchases |

Scenario: Device sends wrong proof
  When agent "C:A" asks device "devC" to charge ".ZZB,whatever" $100 for "goods": "food" at %now
  Then we return error "bad proof"  
  
Scenario: A cashier in the same community asks to charge someone unconfirmed
  When agent "C:A" asks device "devC" to charge ".ZZE,ccE" $100.02 for "goods": "food" at %now
  # cash exchange would be for "cash": "cash out"
  Then we respond ok txid 6 created %now balance "*-101" saying:
  # asterisk means secret balance
  | did     | otherName | amount  | why   |*
  | charged | Eve Five  | $100.02 | goods |
#  Then we return error "not confirmed" with subs:
#  | youName  | inviterName |*
#  | Eve Five | Dee Four    |
