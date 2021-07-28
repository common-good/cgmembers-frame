Feature: Undo
AS a member
I WANT to undo a transaction
SO I don't have to live with my mistakes
 
Setup:
  Given members:
  | uid  | fullName   | floor | acctType    | flags                            | created    | risks   |*
  | .ZZA | Abe One    | -100  | personal    | ok,confirmed,roundup,debt,bankOk | %today-15m | hasBank |
  | .ZZB | Bea Two    | -200  | personal    | ok,confirmed,admin,debt          | %today-15m |         |
  | .ZZC | Corner Pub | -300  | corporation | ok,co                            | %today-15m |         |
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
