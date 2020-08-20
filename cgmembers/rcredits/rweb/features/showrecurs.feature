Feature: Recurring Payments
AS a member
I WANT to review my recurring (and autopay) transactions
SO I can see what they are and terminate them.

Setup:
  Given members:
  | uid  | fullName   | floor | acctType    | flags      | created  |*
  | .ZZA | Abe One    | -100  | personal    | ok,roundup | %now-15m |
  | .ZZB | Bea Two    | -200  | personal    | ok,co      | %now-15m |
  | .ZZC | Corner Pub | -300  | corporation | ok,co      | %now-15m |
  And these "tx_templates":
  | id    | action | from  | to    | amount | purpose    | period  | start    | end        |*
  | 99900 | pay    | .ZZA  | .ZZB  | 59.59  | something  | year    | %now-16m |            |
  | 99901 | pay    | .ZZA  | .ZZC  | 37.00  | this       | quarter | %now-13m |            |
  | 99902 | pay    | .ZZA  | .ZZB  | 43.00  | that       | week    | %now-13m | %now-11m   |
  | 99903 | pay    | .ZZC  | .ZZA  | 37.43  | whatever   | week    | %now-13m |            |
  | 99904 | pay    | .ZZB  | .ZZA  | 22.00  | nothing    | quarter | %now-1m  |            |
  | 99905 | pay    | .ZZB  | .ZZC  | 37.37  | everything | quarter | %now-1m  |            |
  And these "relations":
  | reid | main | other | flags   |*
  | 7773 | .ZZC | .ZZA  | autopay |
  And these "txs":
  | xid | created    | amount | payer | payee | purpose   | taking | recursId |*
  |   2 | %today-16m |  59.59 | .ZZA  | .ZZB  | something |      0 | 99900    |
  |   3 | %today-1m  |  22.00 | .ZZB  | .ZZA  | nothing   |      0 | 99904    |
  |   4 | %today-1d  |  37.43 | .ZZC  | .ZZA  | whatever  |      0 | 99903    |

Scenario: A member looks at their Recurring Payments
  When member ".ZZA" visits page "history/recurring"
  Then we show "Recurring Payments" with:
  |~Way | Who        | Amount | Purpose   | How often? | Starting | Next     | Ending   |~Close   |
  | [R] | Bea Two    | 59.59  | something | Yearly     | %mdY-16m | %mdY     |          | [close] |
  | [R] | Corner Pub | 37.00  | this      | Quarterly  | %mdY-13m | %mdY     |          | [close] |
  | [R] | Bea Two    | 43.00  | that      | Weekly     | %mdY-13m |          | %mdY-11m |         |
  | [L] | Corner Pub | 37.43  | whatever  | Weekly     | %mdY-13m | %mdY+6d  |          | [close] |
  | [L] | Bea Two    | 22.00  | nothing   | Quarterly  | %mdY-1m  | %mdY+2m  |          | [close] |
  | [R] | Corner Pub |        | Invoice   | AutoPay    |          |          |          | [close] |

Scenario: A member stops a recurring transaction
  When member ".ZZA" visits page "history/recurring/recId=99901&do=stop"
  Then we say "status": "recur stopped"
  And we show "Recurring Payments" with:
  | Who        | Amount | How often? | Starting | Next     | Ending   |
  | Bea Two    | 59.59  | Yearly     | %mdY-16m | %mdY     |          |
  | Corner Pub | 37.00  | Quarterly  | %mdY-13m |          | %mdY     |
  | Bea Two    | 43.00  | Weekly     | %mdY-13m |          | %mdY-11m |
  | Corner Pub |        | AutoPay    |          |          |          |

Scenario: A member stops a stopped recurring transaction
  When member ".ZZA" visits page "history/recurring/recId=99902&do=stop"
  Then we say "error": "recur already ended"
  And we show "Recurring Payments" with:
  | Who        | Amount | How often? | Starting | Next     | Ending   |
  | Bea Two    | 59.59  | Yearly     | %mdY-16m | %mdY     |          |
  | Corner Pub | 37.00  | Quarterly  | %mdY-13m | %mdY     |          |
  | Bea Two    | 43.00  | Weekly     | %mdY-13m |          | %mdY-11m |
  | Corner Pub |        | AutoPay    |          |          |          |

Scenario: A member attempts to stop a non-existent recurring transaction
  When member ".ZZA" visits page "history/recurring/recId=99999&do=stop"
  Then we say "error": "invalid recur id"
  And we show "Recurring Payments" with:
  | Who        | Amount | How often? | Starting | Next     | Ending   |
  | Bea Two    | $59.59 | Yearly     | %mdY-16m | %mdY     |          |
  | Corner Pub | $37.00 | Quarterly  | %mdY-13m | %mdY     |          |
  | Bea Two    | $43.00 | Weekly     | %mdY-13m |          | %mdY-11m |
  | Corner Pub |        | AutoPay    |          |          |          |

Scenario: A member attempts to stop another member's recurring transaction
  When member ".ZZA" visits page "history/recurring/recId=99905&do=stop"
  Then we say "error": "recur not yours"
  And we show "Recurring Payments" with:
  | Who        | Amount | How often? | Starting | Next     | Ending   |
  | Bea Two    | 59.59  | Yearly     | %mdY-16m | %mdY     |          |
  | Corner Pub | 37.00  | Quarterly  | %mdY-13m | %mdY     |          |
  | Bea Two    | 43.00  | Weekly     | %mdY-13m |          | %mdY-11m |
  | Corner Pub |        | AutoPay    |          |          |          |

Scenario: A member stops an autopayment
  When member ".ZZA" visits page "history/recurring/reid=7773&do=stop"
  Then we say "status": "recur stopped"
  And we show "Recurring Payments" with:
  | Who        | Amount | How often? | Starting | Next     | Ending   |
  | Bea Two    | 59.59  | Yearly     | %mdY-16m | %mdY     |          |
  | Corner Pub | 37.00  | Quarterly  | %mdY-13m | %mdY     |          |
  | Bea Two    | 43.00  | Weekly     | %mdY-13m |          | %mdY-11m |
  And without:
  | AutoPay | 
  And these "relations":  
  | reid | main | other | flags   |*
  | 7773 | .ZZC | .ZZA  |         |
