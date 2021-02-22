Feature: Accredit
AS a participating business
I WANT to give credit to a customer
SO they can get an automatic rebate when they pay me

Setup:
  Given members:
  | uid  | fullName   | floor | flags             |*
  | .ZZA | Abe One    |  -250 | ok,confirmed,debt |
  | .ZZB | Bea Two    |  -250 | ok,confirmed,debt |
  | .ZZC | Corner Pub |  -250 | ok,confirmed,co   |  
  
  And relations:
  | main | other | permission |*
  | .ZZC | .ZZB  | manage     |

Scenario: A member company gives a customer credit
  When member "C:B" visits page "co/accredit"
  Then we show "Give a Customer Store Credit" with:
  | Customer: |
  | Credit:   |
  
  When member "C:B" confirms "co/accredit" with:
  | customer | amount |*
  | .ZZA     |      3 |
  Then these "tx_rules":
  | id        | 1 |**
  | payerType | %REF_ACCOUNT |
  | payeeType | %REF_ACCOUNT |
  | from      | %MATCH_PAYEE |
  | to        | %MATCH_PAYER |
  | payer     | .ZZA |
  | payee     | .ZZC |
  | amount    |  |
  | portion   | 1 |
  | useMax    |  |
  | amtMax    | 3 |
  | code      | |
  | purpose   | any purchase |
  And we message "store grant|to redeem|want a card|finish get card" to member ".ZZA" with subs:
  | co         | amount | topic           |*
  | Corner Pub | $3     | You got credit! |
  And we message "you gave credit" to member ".ZZC" with subs:
  | nm      | amount | topic                      |*
  | Abe One | $3     | You gave a customer credit |
  
