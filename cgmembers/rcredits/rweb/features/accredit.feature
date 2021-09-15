Feature: Accredit
AS a participating business
I WANT to give credit to a customer
SO they can get an automatic rebate when they pay me

Setup:
  Given members:
  | uid  | fullName | floor | pass | flags             | emailCode |*
  | .ZZA | Abe One  |  -250 | a1   | ok,confirmed,debt | Aa1       |
  | .ZZB | Bea Two  |  -250 | b2   | ok,confirmed,debt | Bb2       |
  | .ZZC | Cor Pub  |  -250 | c3   | ok,confirmed,co   | Cc3       |
  
  And these "u_relations":
  | main | other | permission |*
  | .ZZC | .ZZB  | manage     |

Scenario: A member company gives a customer credit
  When member "C:B" visits page "co/accredit"
  Then we show "Give a Customer Store Credit" with:
  | Customer: |
  | Credit:   |
  
  When member "C:B" confirms "co/accredit" with:
  | customer      | amount |*
  | a@example.com |      3 |
  Then these "tx_rules":
  | id        | 1 |**
  | action    | %ACT_SURTX   |
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
  | co      | amount | topic           |*
  | Cor Pub | $3     | You got credit! |
  And we message "you gave credit" to member ".ZZC" with subs:
  | nm      | amount | topic                      |*
  | Abe One | $3     | You gave a customer credit |
  
Scenario: A member tries to buy credit with credit
  Given these "tx_rules":
  | id        | 1 |**
  | action    | %ACT_SURTX   |
  | payerType | %REF_ACCOUNT |
  | payeeType | %REF_ACCOUNT |
  | from      | %MATCH_PAYEE |
  | to        | %MATCH_PAYER |
  | payer     | .ZZA |
  | payee     | .ZZC |
  | portion   | 1 |
  | amtMax    | 3 |
  | purpose   | any purchase |
  And a button code for:
  | account | secret | for      | amount |*
  | .ZZC    | Cc3    | credit50 | 100.00 |
  When member "?" confirms "cgpay?code=TESTCODE" with:
  | qid  | pass |*
  | .ZZA | a1   |
  Then count "txs" is 1
  And balances:
  | uid  | balance |*
  | .ZZA |    -100 |
  | .ZZC |     100 |
