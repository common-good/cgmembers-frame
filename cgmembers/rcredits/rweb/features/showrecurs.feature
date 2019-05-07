Feature: Recurring transactions
AS a member
I WANT to review my recurring (and autopay) transactions
SO I can see what they are and terminate them.

Setup:
  Given members:
  | uid  | fullName   | floor | acctType    | flags      | created  |*
  | .ZZA | Abe One    | -100  | personal    | ok,roundup | %now-15m |
  | .ZZB | Bea Two    | -200  | personal    | ok,co      | %now-15m |
  | .ZZC | Corner Pub | -300  | corporation | ok,co      | %now-15m |
  And recurs:
  | id    | payer | payee | amount | period | created       | ended      |*
  | 99900 | .ZZA  | .ZZC  | 37.00  | Q      | %daystart-13m | 0          |
  | 99901 | .ZZA  | .ZZB  | 43.00  | W      | %daystart-13m | %now-11m   |
  | 99902 | .ZZA  | .ZZB  | 59.59  | Y      | %daystart-16m | 0          |
  | 99903 | .ZZB  | .ZZA  | 22.00  | Q      | %daystart-13m | 0          |
  | 99904 | .ZZB  | .ZZC  | 37.37  | Q      | %daystart-13m | 0          |
  | 99905 | .ZZC  | .ZZA  | 37.43  | W      | %daystart-13m | 0          |
  And these "relations":
  | reid | main | other | flags   |*
  | 7773 | .ZZC | .ZZA  | autopay |

Scenario: A member looks at their recurring transactions
  When member ".ZZA" visits page "history/recurring"
  Then we show "Recurring Transactions" with:
  | Who        | Amount | How often? | Starting | Next     | Ending   |
  | Corner Pub | 37.00  | Quarterly  | %mdY-13m | ~%mdY+2m |          |
  | Bea Two    | 43.00  | Weekly     | %mdY-13m |          | %mdY-11m |
  | Bea Two    | 59.59  | Yearly     | %mdY-16m | %mdY+8m  |          |
  | Corner Pub |        | AutoPay    | Invoice  |          |          |

Scenario: A member stops a recurring transaction
  When member ".ZZA" visits page "history/recurring/recId=99900&do=stop"
  Then we say "status": "recur stopped"
  And we show "Recurring Transactions" with:
  | Who        | Amount | How often? | Starting | Next     | Ending   |
  | Corner Pub | 37.00  | Quarterly  | %mdY-13m |          | %mdY     |
  | Bea Two    | 43.00  | Weekly     | %mdY-13m |          | %mdY-11m |
  | Bea Two    | 59.59  | Yearly     | %mdY-16m | %mdY+8m  |          |
  | Corner Pub |        | AutoPay    | Invoice  |          |          |

Scenario: A member stops a stopped recurring transaction
  When member ".ZZA" visits page "history/recurring/recId=99901&do=stop"
  Then we say "error": "recur already ended"
  And we show "Recurring Transactions" with:
  | Who        | Amount | How often? | Starting | Next     | Ending   |
  | Corner Pub | 37.00  | Quarterly  | %mdY-13m | ~%mdY+2m |          |
  | Bea Two    | 43.00  | Weekly     | %mdY-13m |          | %mdY-11m |
  | Bea Two    | 59.59  | Yearly     | %mdY-16m | %mdY+8m  |          |
  | Corner Pub |        | AutoPay    | Invoice  |          |          |

Scenario: A member attempts to stop a non-existent recurring transaction
  When member ".ZZA" visits page "history/recurring/recId=99999&do=stop"
  Then we say "error": "invalid recur id"
  And we show "Recurring Transactions" with:
  | Who        | Amount | How often? | Starting | Next     | Ending   |
  | Corner Pub | $37.00 | Quarterly  | %mdY-13m | ~%mdY+2m |          |
  | Bea Two    | $43.00 | Weekly     | %mdY-13m |          | %mdY-11m |
  | Bea Two    | $59.59 | Yearly     | %mdY-16m | %mdY+8m  |          |
  | Corner Pub |        | AutoPay    | Invoice  |          |          |

Scenario: A member attempts to stop another member's recurring transaction
  When member ".ZZA" visits page "history/recurring/recId=99904&do=stop"
  Then we say "error": "recur not yours"
  And we show "Recurring Transactions" with:
  | Who        | Amount | How often? | Starting | Next     | Ending   |
  | Corner Pub | 37.00  | Quarterly  | %mdY-13m | ~%mdY+2m |          |
  | Bea Two    | 43.00  | Weekly     | %mdY-13m |          | %mdY-11m |
  | Bea Two    | 59.59  | Yearly     | %mdY-16m | %mdY+8m  |          |
  | Corner Pub |        | AutoPay    | Invoice  |          |          |

Scenario: A member stops an autopayment
  When member ".ZZA" visits page "history/recurring/reid=7773&do=stop"
  Then we say "status": "recur stopped"
  And we show "Recurring Transactions" with:
  | Corner Pub | 37.00  | Quarterly  | %mdY-13m | ~%mdY+2m |          |
  | Bea Two    | 43.00  | Weekly     | %mdY-13m |          | %mdY-11m |
  | Bea Two    | 59.59  | Yearly     | %mdY-16m | %mdY+8m  |          |
  And without:
  | AutoPay | 
  And these "relations":  
  | reid | main | other | flags   |*
  | 7773 | .ZZC | .ZZA  |         |
