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
  And we notice "refunded you" to member ".ZZB" with subs:
  | created | fullName | otherName | amount | payerPurpose |*
  | %today  | Bea Two  | Abe One   | $123   | bread        |
  And we notice "you refunded" to member ".ZZA" with subs:
  | created | fullName | otherName | amount | payeePurpose |*
  | %today  | Abe One  | Bea Two   | $123   | bread        |

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
  And we notice "refunded you" to member ".ZZC" with subs:
  | created | fullName | otherName | amount | payerPurpose |*
  | %today  | Cor Pub  | Abe One   | $123   | refund       |
  And we notice "you refunded" to member ".ZZA" with subs:
  | created | fullName | otherName | amount | payeePurpose |*
  | %today  | Abe One  | Cor Pub   | $123   | refund       |