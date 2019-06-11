Feature: Undo Transaction
AS a managing agent for an rCredits member company
I WANT to undo the last transaction completed on the POS device I am using
SO I can easily correct a mistake made by another company agent or by me

Summary:
  An agent asks to undo a charge
  An agent asks to undo a refund
  An agent asks to undo a cash in payment
  An agent asks to undo a cash out charge
  An agent asks to undo a charge, with insufficient balance  
  An agent asks to undo a refund, with insufficient balance
  
Setup:
  Given members:
  | uid  | fullName   | email | cc  | cc2  | floor | flags                |*
  | .ZZA | Abe One    | a@    | ccA | ccA2 |  -250 | ok,confirmed,debt    |
  | .ZZB | Bea Two    | b@    | ccB | ccB2 |  -250 | ok,confirmed,debt    |
  | .ZZC | Corner Pub | c@    | ccC |      |  -250 | ok,confirmed,co,debt |
  | .ZZD | Dee Four   | d@    | ccD | ccD2 |     0 | ok,confirmed         |
  | .ZZE | Eve Five   | e@    | ccE | ccE2 |     0 | ok,confirmed,secret  |
  | .ZZF | Far Co     | f@    | ccF |      |     0 | ok,confirmed,co      |
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
  | .ZZC | .ZZA  |   1 | scan       |
  | .ZZC | .ZZB  |   2 | refund     |
  | .ZZC | .ZZD  |   3 | read       |
  | .ZZF | .ZZE  |   1 | sell       |
  Then balances:
  | uid  | balance |*
  | ctty |       0 |
  | .ZZA |       0 |
  | .ZZB |       0 |
  | .ZZC |       0 |

#Variants: with/without an agent
#  | ".ZZA" asks device "devC" | ".ZZC" asks device "codeC" | ".ZZA" $ | ".ZZC" $ | # member to member (pro se) |
#  | ".ZZB" asks device "devC" | ".ZZB" asks device "codeC" | ".ZZA" $ | ".ZZC" $ | # agent to member           |
#  | ".ZZA" asks device "devC" | ".ZZC" asks device "codeC" | ".ZZA" $ | ".ZZC" $ | # member to agent           |
#  | ".ZZB" asks device "devC" | ".ZZB" asks device "codeC" | ".ZZA" $ | ".ZZC" $ | # agent to agent            |

Scenario: An agent asks to undo a charge
  Given transactions: 
  | xid | created | amount | from | to   | purpose      | goods      | taking |*
  | 4   | %now-1d |     80 | .ZZA | .ZZC | whatever     | %FOR_GOODS |      1 |
  When agent "C:B" asks device "devC" to undo transaction with subs:
  | member | code | amount | goods | description | created |*
  | .ZZA   | ccA  | 80.00  |     1 | whatever    | %now-1d |
#  When agent "C:B" asks device "devC" to undo transaction 4 code "ccA"
  Then we respond ok txid 5 created %now balance 0 saying:
  | solution | did      | otherName | amount | why   |*
  | reversed | refunded | Abe One   | $80    | goods |
  And with did ""
  And with undo "4"
  And we notice "new refund" to member ".ZZA" with subs:
  | created | otherName  | amount | payerPurpose |*
  | %now    | Corner Pub | $80    | whatever     |

Scenario: An agent asks to undo a charge when balance is secret
  Given transactions: 
  | xid | created   | amount | from | to   | purpose      | taking |*
  | 5   | %today-1d |     80 | .ZZE | .ZZC | whatever     |      1 |
  When agent "C:B" asks device "devC" to undo transaction 5 code "ccE"
  Then we respond ok txid 6 created %now balance "*0" saying:
  | solution | did      | otherName | amount | why   |*
  | reversed | refunded | Eve Five  | $80    | goods |
  And with did ""
  And with undo "5"
  And we notice "new refund" to member ".ZZE" with subs:
  | created | otherName  | amount | payerPurpose |*
  | %today  | Corner Pub | $80    | whatever     |

Scenario: An agent asks to undo a refund
  Given transactions: 
  | xid | created   | amount | from | to   | purpose      | taking |*
  | 4   | %today-1d |    -80 | .ZZA | .ZZC | refund       |      1 |
  When agent "C:B" asks device "devC" to undo transaction 4 code "ccA"
  Then we respond ok txid 5 created %now balance 0 saying:
  | solution | did        | otherName | amount | why   |*
  | reversed | re-charged | Abe One   | $80    | goods |
  And with did ""
  And with undo "4"
  And we notice "new charge" to member ".ZZA" with subs:
  | created | otherName  | amount | payerPurpose |*
  | %today  | Corner Pub | $80    | refund       |

Scenario: An agent asks to undo a cash-out charge
  Given transactions: 
  | xid | created   | amount | from | to   | purpose  | goods      | taking |*
  | 4   | %today-1d |     80 | .ZZA | .ZZC | cash out | %FOR_USD |      1 |
  When agent "C:B" asks device "devC" to undo transaction 4 code "ccA"
  Then we respond ok txid 5 created %now balance 0 saying:
  | solution | did      | otherName | amount | why |*
  | reversed | credited | Abe One   | $80    | usd |
  And with did ""
  And with undo "4"
  And we notice "new payment" to member ".ZZA" with subs:
  | created | fullName | otherName  | amount | payeePurpose |*
  | %today  | Abe One  | Corner Pub | $80    | cash out     |

Scenario: An agent asks to undo a cash-in payment
  Given transactions: 
  | xid | created   | amount | from | to   | purpose | goods      | taking |*
  | 4   | %today-1d |    -80 | .ZZA | .ZZC | cash in | %FOR_USD |      1 |
  When agent "C:B" asks device "devC" to undo transaction 4 code "ccA"
  Then we respond ok txid 5 created %now balance 0 saying:
  | solution | did        | otherName | amount | why |*
  | reversed | re-charged | Abe One   | $80    | usd |
  And with did ""
  And with undo "4"
  And we notice "new charge" to member ".ZZA" with subs:
  | created | fullName | otherName  | amount | payerPurpose |*
  | %today  | Abe One  | Corner Pub | $80    | cash in      |

Scenario: An agent asks to undo a charge, with insufficient balance  
  Given transactions: 
  | xid | created   | amount | from | to   | purpose      | goods        | taking |*
  | 4   | %today-1d |     80 | .ZZA | .ZZC | whatever     | %FOR_GOODS |      1 |
  | 5   | %today    |    300 | .ZZC | .ZZB | cash out     | %FOR_USD   |      0 |
  When agent "C:B" asks device "devC" to undo transaction 4 code "ccA"
  Then we respond ok txid 6 created %now balance 0 saying:
  | solution | did      | otherName | amount | why   |*
  | reversed | refunded | Abe One   | $80    | goods |
  And with did ""
  And with undo "4"
  And we notice "new refund" to member ".ZZA" with subs:
  | created | otherName  | amount | payerPurpose |*
  | %today  | Corner Pub | $80    | whatever     |
  And balances:
  | uid  | balance |*
  | ctty |       0 |
  | .ZZA |       0 |
  | .ZZB |     300 |
  | .ZZC |    -300 |

Scenario: An agent asks to undo a refund, with insufficient balance  
  Given transactions: 
  | xid | created   | amount | from | to   | purpose      | goods        | taking |*
  | 4   | %today-1d |    -80 | .ZZA | .ZZC | refund       | %FOR_GOODS |      1 |
  | 5   | %today    |    300 | .ZZA | .ZZB | cash out     | %FOR_USD   |      0 |
  When agent "C:B" asks device "devC" to undo transaction 4 code "ccA"
  Then we respond ok txid 6 created %now balance -300 saying:
  | solution | did        | otherName | amount | why   |*
  | reversed | re-charged | Abe One   | $80    | goods |
  And with did ""
  And with undo "4"
  And we notice "new charge" to member ".ZZA" with subs:
  | created | otherName  | amount | payerPurpose |*
  | %today  | Corner Pub | $80    | refund       |
  And balances:
  | uid  | balance |*
  | ctty |       0 |
  | .ZZA |    -300 |
  | .ZZB |     300 |
  | .ZZC |       0 |

Scenario: An agent asks to undo a charge, without permission
  Given transactions: 
  | xid | created   | amount | from | to   | purpose      | goods        | taking |*
  | 4   | %today-1d |     80 | .ZZB | .ZZC | whatever     | %FOR_GOODS |      1 |
  When agent "C:A" asks device "devC" to undo transaction 4 code "ccB"
  Then we respond ok txid 5 created %now balance 0 saying:
  | solution | did      | otherName | amount | why   |*
  | reversed | refunded | Bea Two   | $80    | goods |
#  Then we return error "no perm" with subs:
#  | what    |*
#  | refunds |
#  And we notice "bad forced tx|no perm" to member ".ZZC" with subs:
#  | what    | account | amount | created | by                  |*
#  | refunds | Bea Two | $80    | %dmy-1d | %(" agent Abe One") |

Scenario: An agent asks to undo a refund, without permission
  Given transactions: 
  | xid | created   | amount | from | to   | purpose      | goods        | taking |*
  | 4   | %today-1d |    -80 | .ZZB | .ZZC | refund       | %FOR_GOODS |      1 |
  When agent "C:D" asks device "devC" to undo transaction 4 code "ccB"
  Then we respond ok txid 5 created %now balance 0 saying:
  | solution | did        | otherName | amount | why   |*
  | reversed | re-charged | Bea Two   | $80    | goods |
#  Then we return error "no perm" with subs:
#  | what  |*
#  | sales |

Scenario: An agent asks to undo a non-existent transaction
#  When agent "C:A" asks device "devC" to undo transaction 99 code %whatever
  When agent "C:B" asks device "devC" to undo transaction with subs:
  | member | code | amount | goods | description   | created   |*
  | .ZZA   | ccA  | 80.00  |     1 | neverhappened | %today-1d |
  Then we respond ok txid 0 created "" balance 0
  And with did ""
  And with undo ""

Scenario: A cashier reverses a transaction with insufficient funds
  Given transactions: 
  | xid | created   | amount | from | to   | purpose |*
  | 4   | %today-1m |    100 | ctty | .ZZC | jnsaqwa |
  And agent "C:B" asks device "devC" to charge ".ZZA,ccA" $-100 for "cash": "cash in" at "%now-1h" force 0
  Then transaction headers: 
  | xid | created | actorId | actorAgentId |*
  | 5   | %now-1h |    .ZZC |         .ZZB |
  And transaction entries:
  | xid | entryType    | amount | uid  | agentUid | description | acctTid |*
  |   5 | %ENTRY_PAYER |    100 | .ZZA | .ZZA     | cash in     | 1       |
  |   5 | %ENTRY_PAYEE |   -100 | .ZZC | .ZZB     | cash in     | 2       |
  Given transactions:
  | xid | created | amount | from | to   | purpose |*
  | 6   | %today  |      1 | .ZZA | .ZZB | cash    |
  When agent "C:B" asks device "devC" to charge ".ZZA,ccA" $-100 for "cash": "cash in" at "%now-1h" force -1
  Then we respond ok txid 7 created %now balance -1
#  And with proof of agent "C:B" amount -100.00 created "%now-1h" member ".ZZA" code "ccA"
  And with undo "5"
  And we notice "new charge" to member ".ZZA" with subs:
  | created | fullName | otherName  | amount | payerPurpose |*
  | %today  | Bea Two  | Corner Pub | $100   | cash in      |
  And balances:
  | uid  | balance |*
  | ctty |    -100 |
  | .ZZA |      -1 |
  | .ZZB |       1 |
  | .ZZC |     100 |
