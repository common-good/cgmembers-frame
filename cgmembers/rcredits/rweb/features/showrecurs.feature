Feature: Automated Payments
AS a member
I WANT to review my recurring (and autopay) transactions
SO I can see what they are and terminate them.

Setup:
  Given members:
  | uid  | fullName   | floor | acctType    | flags            | created  |*
  | .ZZA | Abe One    | -100  | personal    | ok,roundup,admin | %now-15m |
  | .ZZB | Bea Two    | -200  | personal    | ok,co            | %now-15m |
  | .ZZC | Corner Pub | -300  | corporation | ok,co            | %now-15m |
  And these "people":
  | pid | fullName |*
  | 123 | Ned Nine |
  And these "tx_timed":
  | id    | action | from         | to    | amount | purpose    | period  | start    | end        | payerType    | payer |*
  | 99900 | pay    | .ZZA         | .ZZB  | 59.59  | something  | year    | %now-16m |            | %REF_ANYBODY |       |
  | 99901 | pay    | .ZZA         | .ZZC  | 37.00  | this       | quarter | %now-13m |            | %REF_ANYBODY |       |
  | 99902 | pay    | .ZZA         | .ZZB  | 43.00  | that       | week    | %now-13m | %now-11m   | %REF_ANYBODY |       |
  | 99903 | pay    | .ZZC         | .ZZA  | 37.43  | whatever   | week    | %now-13m |            | %REF_ANYBODY |       |
  | 99904 | pay    | .ZZB         | .ZZA  | 22.00  | nothing    | quarter | %now-1m  |            | %REF_ANYBODY |       |
  | 99905 | pay    | .ZZB         | .ZZC  | 37.37  | everything | quarter | %now-1m  |            | %REF_ANYBODY |       |
  | 99906 | pay    | %MATCH_PAYER | .ZZA  | 99.87  | donation   | week    | %now-2d  |            | %REF_PERSON  | 123   |
  And these "relations":
  | reid | main | other | flags   |*
  | 7773 | .ZZC | .ZZA  | autopay |
  And these "txs":
  | xid | created  | amount | payer      | payee | purpose   | taking | recursId |*
  |   2 | %now-16m |  59.59 | .ZZA       | .ZZB  | something |      0 | 99900    |
  |   3 | %now-1m  |  22.00 | .ZZB       | .ZZA  | nothing   |      0 | 99904    |
  |   4 | %now-1d  |  37.43 | .ZZC       | .ZZA  | whatever  |      0 | 99903    |
  |   5 | %now-2d  |  99.87 | %UID_OUTER | .ZZA  | donation  |      0 | 99906    |
  And these "txs2":
  | xid | created | completed | amount | payee | pid |*
  | 7   | %now-2d | %now-23d  | 99.87  | .ZZA  | 123 |

Scenario: A member looks at their Recurring Payments
  When member ".ZZA" visits page "history/recurring"
  Then we show "Automated Payments" with:
  |~Way | Who        | Amount | Purpose   | How often? | Starting | Next     | Ending   |~Close   |
  | [R] | Corner Pub | 37.00  | this      | Quarterly  | %mdY-13m | %mdY     |          | [close] |
  | [R] | Bea Two    | 59.59  | something | Yearly     | %mdY-16m | %mdY     |          | [close] |
  | [L] | Ned Nine   | 99.87  | donation  | Weekly     | %mdY-2d  | %mdY+5d  |          | [close] |
  | [L] | Bea Two    | 22.00  | nothing   | Quarterly  | %mdY-1m  | %mdY+2m  |          | [close] |
  | [L] | Corner Pub | 37.43  | whatever  | Weekly     | %mdY-13m | %mdY+6d  |          | [close] |
  | [R] | Bea Two    | 43.00  | that      | Weekly     | %mdY-13m |          | %mdY-11m |         |
  | [R] | Corner Pub |        | Invoice   | AutoPay    |          |          |          | [close] |
  
Scenario: A non-admin looks at their Recurring Payments
  Given members have:
  | uid  | flags      |*
  | .ZZA | ok,roundup |
  # not admin so don't show closed items
  When member ".ZZA" visits page "history/recurring"
  Then we show "Automated Payments" with:
  |~Way | Who        | Amount | Purpose   | How often? | Starting | Next     | Ending   |~Close   |
  | [R] | Corner Pub | 37.00  | this      | Quarterly  | %mdY-13m | %mdY     |          | [close] |
  | [R] | Bea Two    | 59.59  | something | Yearly     | %mdY-16m | %mdY     |          | [close] |
  | [L] | Ned Nine   | 99.87  | donation  | Weekly     | %mdY-2d  | %mdY+5d  |          | [close] |
  | [L] | Bea Two    | 22.00  | nothing   | Quarterly  | %mdY-1m  | %mdY+2m  |          | [close] |
  | [L] | Corner Pub | 37.43  | whatever  | Weekly     | %mdY-13m | %mdY+6d  |          | [close] |
  | [R] | Corner Pub |        | Invoice   | AutoPay    |          |          |          | [close] |
  And without:
  | 43.00  | that      |

Scenario: A member stops a recurring transaction
  When member ".ZZA" visits page "history/recurring/recId=99901&do=stop"
  Then we say "status": "recur stopped"
  And we show "Automated Payments" with:
  | Who        | Amount | How often? | Starting | Next     | Ending   |
  | Bea Two    | 59.59  | Yearly     | %mdY-16m | %mdY     |          |
  | Bea Two    | 43.00  | Weekly     | %mdY-13m |          | %mdY-11m |
  | Corner Pub | 37.00  | Quarterly  | %mdY-13m |          | %mdY     |
  | Corner Pub |        | AutoPay    |          |          |          |

Scenario: A member stops a stopped recurring transaction
  When member ".ZZA" visits page "history/recurring/recId=99902&do=stop"
  Then we say "error": "recur already ended"
  And we show "Automated Payments" with:
  | Who        | Amount | How often? | Starting | Next     | Ending   |
  | Corner Pub | 37.00  | Quarterly  | %mdY-13m | %mdY     |          |
  | Bea Two    | 59.59  | Yearly     | %mdY-16m | %mdY     |          |
  | Bea Two    | 43.00  | Weekly     | %mdY-13m |          | %mdY-11m |
  | Corner Pub |        | AutoPay    |          |          |          |

Scenario: A member attempts to stop a non-existent recurring transaction
  When member ".ZZA" visits page "history/recurring/recId=99999&do=stop"
  Then we say "error": "invalid recur id"
  And we show "Automated Payments" with:
  | Who        | Amount | How often? | Starting | Next     | Ending   |
  | Corner Pub | $37.00 | Quarterly  | %mdY-13m | %mdY     |          |
  | Bea Two    | $59.59 | Yearly     | %mdY-16m | %mdY     |          |
  | Bea Two    | $43.00 | Weekly     | %mdY-13m |          | %mdY-11m |
  | Corner Pub |        | AutoPay    |          |          |          |

Scenario: A member attempts to stop another member's recurring transaction
  When member ".ZZA" visits page "history/recurring/recId=99905&do=stop"
  Then we say "error": "recur not yours"
  And we show "Automated Payments" with:
  | Who        | Amount | How often? | Starting | Next     | Ending   |
  | Corner Pub | 37.00  | Quarterly  | %mdY-13m | %mdY     |          |
  | Bea Two    | 59.59  | Yearly     | %mdY-16m | %mdY     |          |
  | Bea Two    | 43.00  | Weekly     | %mdY-13m |          | %mdY-11m |
  | Corner Pub |        | AutoPay    |          |          |          |

Scenario: A member stops an autopayment
  When member ".ZZA" visits page "history/recurring/reid=7773&do=stop"
  Then we say "status": "recur stopped"
  And we show "Automated Payments" with:
  | Who        | Amount | How often? | Starting | Next     | Ending   |
  | Corner Pub | 37.00  | Quarterly  | %mdY-13m | %mdY     |          |
  | Bea Two    | 59.59  | Yearly     | %mdY-16m | %mdY     |          |
  | Bea Two    | 43.00  | Weekly     | %mdY-13m |          | %mdY-11m |
  And without:
  | AutoPay | 
  And these "relations":  
  | reid | main | other | flags   |*
  | 7773 | .ZZC | .ZZA  |         |
