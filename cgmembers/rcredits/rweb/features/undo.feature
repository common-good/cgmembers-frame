Feature: Undo
AS a member
I WANT to undo a transaction
SO I don't have to live with my mistakes
 
Setup:
  Given members:
  | uid  | fullName | floor | acctType    | flags                            | created    | risks   | helper |*
  | .ZZA | Abe One  | -100  | personal    | ok,confirmed,roundup,debt,bankOk | %today-15m | hasBank | 0      |
  | .ZZB | Bea Two  | -200  | personal    | ok,confirmed,admin,debt          | %today-15m |         | .ZZA   |
  | .ZZC | Cor Pub  | -300  | corporation | ok,co                            | %today-15m |         | .ZZB   |
  And member ".ZZB" has admin permissions: "seeAccts"
  And these "txs":
  | xid | amt | uid1 | uid2 | purpose |*
  |   2 | 123 | .ZZB | .ZZA | bread   |

Scenario: A member reverses a payment from someone
  When member ".ZZA" visits "history/transactions/period=365&undo=2"
  Then these "txs":
  | xid | amt  | uid1 | uid2 | purpose | reversesXid |*
  |   2 |  123 | .ZZB | .ZZA | bread   |             |
  |   3 | -123 | .ZZB | .ZZA | bread   |           2 |
  And we say "status": "report undo|tx desc active" with subs:
  | solution | did      | otherName | amount |*
  | reversed | refunded | Bea Two   | $123   |  
  And we message "refunded you" to member ".ZZB" with subs:
  | created | otherName | amount | payerPurpose |*
  | %today  | Abe One   | $123   | bread        |
  And we message "you refunded" to member ".ZZA" with subs:
  | created | otherName | amount | payeePurpose |*
  | %today  | Bea Two   | $123   | bread        |

Scenario: A member reverses a payment to someone
  Given these "u_relations":
  | main | other | permission |*
  | .ZZC | .ZZB  | scan       |
  And these "txs":
  | xid | amt  | uid1 | uid2 | agt1 | agt2 | purpose | actorId | actorAgentId |*
  |   3 | -123 | .ZZA | .ZZC | .ZZA | .ZZB | labor   | .ZZC    | .ZZB         |
  When member "A:1" visits "history/transactions/period=365&undo=3"
  Then these "txs":
  | xid | amt  | uid1 | uid2 | agt1 | agt2 | purpose | actorId | reversesXid |*
  |   3 | -123 | .ZZA | .ZZC | .ZZA | .ZZB | labor   | .ZZC    |             |
  |   4 |  123 | .ZZA | .ZZC | .ZZA | .ZZB | labor   | .ZZA    | 3           |
  And we say "status": "report undo|tx desc active" with subs:
  | solution | did      | otherName | amount |*
  | reversed | refunded | Cor Pub   | $123   |  
  And we message "refunded you" to member ".ZZC" with subs:
  | created | otherName | amount | payerPurpose |*
  | %today  | Abe One   | $123   | labor        |
  And we message "you refunded" to member ".ZZA" with subs:
  | created | otherName | amount | payeePurpose |*
  | %today  | Cor Pub   | $123   | labor        |
  
Scenario: A customer reverses a refund from a store
  Given these "u_relations":
  | main | other | permission |*
  | .ZZC | .ZZB  | scan       |
  And these "txs":
  | xid | amt  | uid1 | uid2 | agt1 | agt2 | purpose | actorId | actorAgentId |*
  |   3 | -123 | .ZZA | .ZZC | .ZZA | .ZZB | refund  | .ZZC    | .ZZB         |
  When member "A:1" visits "history/transactions/period=365&undo=3"
  Then these "txs":
  | xid | amt  | uid1 | uid2 | agt1 | agt2 | purpose | actorId | reversesXid |*
  |   3 | -123 | .ZZA | .ZZC | .ZZA | .ZZB | refund  | .ZZC    |             |
  |   4 |  123 | .ZZA | .ZZC | .ZZA | .ZZB | refund  | .ZZA    | 3           |
  And we say "status": "report undo|tx desc active" with subs:
  | solution | did      | otherName | amount |*
  | reversed | refunded | Cor Pub   | $123   |  
  And we message "refunded you" to member ".ZZC" with subs:
  | created | otherName | amount | payerPurpose |*
  | %today  | Abe One   | $123   | refund       |
  And we message "you refunded" to member ".ZZA" with subs:
  | created | otherName | amount | payeePurpose |*
  | %today  | Cor Pub   | $123   | refund       |
  
Scenario: A member tries to reverse a non-existent transaction
  When member ".ZZA" visits "history/transactions/period=365&undo=3"
  Then we say "error": "no such tx"
  And count "txs" is 1

Scenario: An administrator reverses a bank transfer in
  Given these "txs2":
  | txid | payee | amount | created    | completed  | deposit    |*
  |   11 |  .ZZA |   1000 | %today-13m | %today-13m | %today-13m |
  Then these "txs": 
  | xid | created    | amount | payer   | payee | purpose   |*
  |   3 | %today-13m |   1000 | bank | .ZZA  | from bank |
  When member "A:1" visits "history/transactions/period=365&undo=3"
  Then we say "status": "reversed bank tx" with subs:
  | amount | who     |*
  | $1,000 | Abe One |
  And we message "bank tx canceled" to member ".ZZA" with subs:
  | xid | 4 |**
  And these "txs2":
  | txid | payee | amount | created | completed  | deposit           | xid |*
  |  -11 |  .ZZA |  -1000 | %now    | %today-13m | %(%today-13m + 1) |   4 |
  And these "txs":
  | xid | created | amount | payer   | payee | purpose                  |*
  |   4 | %now    |  -1000 | bank | .ZZA  | bank transfer adjustment |

Scenario: An administrator reverses a bank transfer out
  Given these "txs2":
  | txid | payee | amount | created    | completed  | deposit    |*
  |   11 |  .ZZA |  -1000 | %today-13m | %today-13m | %today-13m |
  Then these "txs": 
  | xid | created    | amount | payer | payee | purpose |*
  |   3 | %today-13m |  -1000 | bank  | .ZZA  | to bank |
  When member "A:1" visits "history/transactions/period=365&undo=3"
  Then we say "status": "reversed bank tx" with subs:
  | amount  | who     |*
  | $-1,000 | Abe One |
  And we message "bank tx canceled" to member ".ZZA" with subs:
  | xid | 4 |**
#  This should be an immediate notice
  And these "txs2":
  | txid | payee | amount | created | completed  | deposit           | xid |*
  |  -11 |  .ZZA |   1000 | %now    | %today-13m | %(%today-13m + 1) |   4 |
  And these "txs":
  | xid | created | amount | payer | payee | purpose                  |*
  |   4 | %now    |   1000 | bank  | .ZZA  | bank transfer adjustment |

Scenario: An administrator reverses a non-member ACH in
  Given these "txs":
  | eid | xid | payer      | payee | amount | purpose | cat1        | cat2        | type     |*
  |   3 | 3   | %UID_OUTER | .ZZC  | 100    | grant   |             | D-FBO       | %E_OUTER |
  |   4 | 3   | .ZZC       | cgf   | 5      | sponsor | D-FBO       | FS-FEE      | %E_AUX   |
  And these "txs2":
  | txid | xid | payee | amount | completed | deposit | pid |*
  |   11 | 3   | .ZZC  | 100    | %now      | %now    | 2   |
  And these "people":
  | pid | fullName | address | city | state | zip   |*
  | 2   | Dee Forn | 4 Fr St | Fton | MA    | 01004 |
  When member "C:1" visits "history/transactions/period=365&undo=3"
  Then we say "status": "reversed bank tx" with subs:
  | amount | who     |*
  | $100   | Cor Pub |
#  We should tell Cor Pub and Dee Forn immediately
  And these "txs2":
  | txid | xid | payee | amount | created | completed | deposit | pid |*
  |  -11 | 4   | .ZZC  | -100   | %now    | %now      | %now    | 2   |
  And these "txs":
  | eid | xid | payer      | payee | amount | purpose  | cat1        | cat2        | type     |*
  |   5 | 4   | %UID_OUTER | .ZZC  | -100   | grant    |             | D-FBO       | %E_OUTER |
  |   6 | 4   | .ZZC       | cgf   | -5     | sponsor  | D-FBO       | FS-FEE      | %E_AUX   |
