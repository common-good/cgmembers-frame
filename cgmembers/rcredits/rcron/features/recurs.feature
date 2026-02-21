Feature: Recurs
AS a member
I WANT to make recurring payments (typically gifts) to other members
SO I can save on memory and labor.

Setup:
  Given members:
  | uid  | fullName | phone        | city  | state | zip   | country | postalAddr | flags               | bankAccount | floor |*
  | .ZZA | Abe One  | 413-111-1111 | Atown | AK    | 01000 | US      | 1 A, A, AK | ok,confirmed,bankOk | USkk9000001 |   -20 |
  | .ZZB | Bea Two  | 413-222-2222 | Btown | PA    | 01002 | US      | 2 B, B, BC | ok,confirmed        |             |  -200 |
  | .ZZC | Cor Pub  | 413-333-3333 | Ctown | CT    | 03000 | US      | 3 C, C, CT | ok,co,confirmed     |             |     0 |
  And these "txs":
  | xid | created | amount | payer | payee | purpose |*
  |   1 | %now-4m |    100 | .ZZB | .ZZA | loan    |
  Then balances:
  | uid  | balance |*
  | .ZZA | 100     |

Scenario: A brand new recurring payment can be completed
  Given these "tx_timed":
  | action | start      | from | to   | amount | period | purpose |*
  | pay    | %yesterday | .ZZA | .ZZB |     10 | week   | pmt     |
  When cron runs "recurs"
  Then these "txs":
  | xid | created | amount | payer | payee | purpose | flags |*
  |   2 | %now    |     10 | .ZZA  | .ZZB  | pmt     | self  |
  And we message "paid you" to member ".ZZB" with subs:
  | otherName | amount | payeePurpose | aPayLink |*
  | Abe One   | $10    | pmt          | ?        |
  And we message "recur pay" to member ".ZZA" with subs:
  | amount | when   | purpose | payee   |*
  |    $10 | weekly | pmt     | Bea Two |
  # and many other fields
  And count "txs" is 2
  And count "txs2" is 0
  And count "tx_requests" is 0
  When cron runs "recurs"
  Then count "txs" is 2
  And count "txs2" is 0
  And count "tx_requests" is 0
  
Scenario: A new recurring payment is not to be completed yet
  Given these "tx_timed":
  | action | start      | from | to   | amount | period | purpose |*
  | pay    | %tomorrow | .ZZA | .ZZB |     10 | week   | pmt     |
  Then count "txs" is 1
  When cron runs "recurs"
  Then count "txs" is 1
  And count "tx_requests" is 0

Scenario: A recurring sweep can be completed
  Given these "tx_timed":
  | id | action | start      | from | to   | amount | period | purpose |*
  |  7 | pay    | %yesterday | .ZZA | bank |   %NUL | week   | pmt     |
  When cron runs "recurs"
  Then these "txs2":
  | xid | created | amount | payee | completed | deposit |*
  |   2 | %now    |   -100 | .ZZA  |      %now |       0 |
  And these "txs":
  | xid | created | amount | payer | payee | recursId | for2    |*
  |   2 | %now    |   -100 | bank  | .ZZA  |        7 | to bank |
  And we message "banked" to member ".ZZA" with subs:
  | action  | tofrom | amount | why                              |*
  | deposit | to     | $100   | (your automatic weekly transfer) |

Scenario: A recurring bank transfer can be completed
  Given these "tx_timed":
  | id | action | start      | from | to   | amount | period | purpose |*
  |  7 | pay    | %yesterday | .ZZA | bank |     50 | day    | to bank |
  When cron runs "recurs"
  Then these "txs2":
  | xid | created | amount | payee | completed | deposit |*
  |   2 | %now    |    -50 | .ZZA  |      %now |       0 |
  And these "txs":
  | xid | created | amount | payer | payee | recursId | for2    |*
  |   2 | %now    |    -50 | bank  | .ZZA  |        7 | to bank |
  And count "txs" is 2
  And we message "banked" to member ".ZZA" with subs:
  | action  | tofrom | amount | why                             |*
  | deposit | to     | $50    | (your automatic daily transfer) |
  
  Given it's later
  When cron runs "recurs"
  Then count "txs" is 2
  
Scenario: A recurring bank transfer fails for insufficient funds
  Given these "tx_timed":
  | id | action | start      | from | to   | amount | period | purpose |*
  |  7 | pay    | %yesterday | .ZZA | bank |    200 | day    | to bank |
  When cron runs "recurs"
  Then we message "auto bankout nsf" to member ".ZZA" with subs:
  | when  | avail | amount |*
  | daily | $100  | $200   |

Scenario: A second recurring payment can be completed
  Given these "tx_timed":
  | id | action | start    | from | to   | amount | period | purpose |*
  |  8 | pay    | %now0-8d | .ZZA | .ZZB |     10 | week   | pmt     |
  And these "txs":
  | xid | created | amount | payer | payee | purpose | flags | recursId |*
  |   2 | %now-8d |     10 | .ZZA  | .ZZB  | pmt     | self  |        8 |
  When cron runs "recurs"
  Then these "txs":
  | xid | created | amount | payer | payee | purpose | flags | recursId |*
  |   3 | %now    |     10 | .ZZA  | .ZZB  | pmt     | self  |        8 |

Scenario: A second recurring payment can be completed from a non-member by ACH
  Given these "people":
  | pid | fullName |*
  | 123 | Ned Nine |
  And these "tx_timed":
  | id | action | start    | from         | to   | amount | period | purpose | payerType   | payer | stripeId | flags |*
  |  8 | pay    | %now0-8d | %MATCH_PAYER | .ZZB |     10 | week   | pmt     | %REF_PERSON | 123   |          | gift  |
  And these "txs":
  | xid | created | amount | payer      | payee | purpose | flags | recursId | type     |*
  |   2 | %now-8d |     10 | %UID_OUTER | .ZZB  | pmt     | self  |        8 | %E_OUTER |
  And these "txs2":
  | xid | created | completed | amount | payee | pid | bankAccount | isSavings |*
  | 2   | %now-8d | %now-8d   | 10     | .ZZB  | 123 | USkk9000001 | %NUL      |
  When cron runs "recurs"
  Then these "txs":
  | xid | created | amount | payer      | payee | purpose | flags     | recursId | type     |*
  |   3 | %now    |     10 | %UID_OUTER | .ZZB  | pmt     | self,gift |        8 | %E_OUTER |
  And these "txs2":
  | xid | created | completed | amount | payee | pid | bankAccount | isSavings |*
  | 3   | %now    | %now      | 10     | .ZZB  | 123 | USkk9000001 | %NUL      |

Scenario: A second recurring payment can be completed from a non-member by card
  Given these "people":
  | pid | fullName |*
  | 123 | Ned Nine |
  And these "tx_timed":
  | id | action | start    | from         | to   | amount | period | purpose | payerType   | payer | stripeId |*
  |  8 | pay    | %now0-8d | %MATCH_PAYER | .ZZB |     10 | week   | pmt     | %REF_PERSON | 123   | strId456 |
  And these "txs":
  | xid | created | amount | payer      | payee | purpose | flags  | recursId | type     |*
  |   2 | %now-8d |     10 | %UID_OUTER | .ZZB  | pmt     |        |        8 | %E_OUTER |
  And these "txs2":
  | xid | created | completed | amount | payee | pid | bankAccount | isSavings |*
  | 2   | %now-8d | %now-8d   | 10     | .ZZB  | 123 | %NUL        | %NUL      |
  When cron runs "recurs"
  Then these "txs":
  | xid | created | amount | payer      | payee | purpose | flags  | recursId | type     |*
  |   3 | %now    |     10 | %UID_OUTER | .ZZB  | pmt     | self   |        8 | %E_OUTER |
  And these "txs2":
  | xid | created | completed | amount | payee | pid | bankAccount | isSavings |*
  | 3   | %now    | %now      | 10     | .ZZB  | 123 | %NUL        | %NUL      |
  
Scenario: A second recurring payment from a non-member by card fails
  Given these "people":
  | pid | fullName | email |*
  | 123 | Ned Nine | 9@    |
  And next cc payment will fail
  And these "tx_timed":
  | id | action | start    | from         | to   | amount | period | purpose | payerType   | payer | stripeId |*
  |  8 | pay    | %now0-8d | %MATCH_PAYER | .ZZC |     10 | week   | pmt     | %REF_PERSON | 123   | strId456 |
  And these "txs":
  | xid | created | amount | payer      | payee | purpose | flags  | recursId | type     |*
  |   2 | %now-8d |     10 | %UID_OUTER | .ZZC  | pmt     |        |        8 | %E_OUTER |
  And these "txs2":
  | xid | created | completed | amount | payee | pid | bankAccount | isSavings |*
  | 2   | %now-8d | %now-8d   | 10     | .ZZC  | 123 | %NUL        | %NUL      |
  When cron runs "recurs"
  Then we tell admin "Stripe charge failed" with subs:
  | amount |*
  | 10     |
  And we email "cc-payment-failed" to member "9@example.com" with subs:
  | noFrame       | 1                                    |**
  | date          | %mdY                                 |
  | amount        | $10                                  |
  | qid           | .ZZC                                 |
  | coName        | Cor Pub                              |
  | coPostalAddr  | 3 C, C, CT                           |
  | coPhone       | +1 413 333 3333                      |
  | erMsg         | invalid card                         |
  | fullName      | Ned Nine                             |
  | howOften      | weekly                               |
  | recurCanceled | All future recurrences are canceled. |
  | giftLink      | ?                                    |
  | site          | %BASE_URL                            |
  | orgPhone      | %CGF_PHONE                           |
  | emailCoded    | %60%21PYylzG3tpygMrwwtrgw            |
  And these "tx_timed":
  | id | end  |*
  | 8  | %now |

Scenario: A recurring payment happened yesterday
  Given these "tx_timed":
  | id | action | start      | from | to   | amount | period | purpose |*
  |  8 | pay    | %yesterday | .ZZA | .ZZC |     10 | month  | pmt     |
  And these "txs":
  | xid | created    | amount | payer | payee | purpose | flags | recursId |*
  |   2 | %yesterday |     10 | .ZZA  | .ZZC  | pmt     | self  |        8 |
  When cron runs "recurs"
  Then count "txs" is 2
  
Scenario: A recurring payment happened long enough ago to repeat
  Given these "tx_timed":
  | id | start         | from | to   | amount | period | purpose |*
  |  8 | %yesterday-1w | .ZZA | .ZZC |     10 | week   | pmt     |
  And these "txs":
  | xid | created       | amount | payer | payee | purpose | flags | recursId |*
  |   2 | %yesterday-1w |     10 | .ZZA  | .ZZC  | pmt     | self  |        8 |
  When cron runs "recurs"
  Then these "txs":
  | xid | created    | amount | payer | payee | purpose | flags | recursId |*
  |   3 | %now       |     10 | .ZZA  | .ZZC  | pmt     | self  |        8 |
  And count "txs" is 3
  And count "tx_requests" is 0
  
Scenario: A delayed payment does not happen immediately
  Given these "tx_timed":
  | id | start   | from | to   | amount | period | purpose |*
  |  8 | %now+1w | .ZZA | .ZZC |     10 | week   | pmt     |
  Then count "txs" is 1
  When cron runs "recurs"
  Then count "txs" is 1
  
Scenario: A recurring payment cannot be completed
  Given these "tx_timed":
  | id | start      | from | to   | amount | period | purpose |*
  |  8 | %yesterday | .ZZA | .ZZB |    200 | week   | pmt     |
  When cron runs "recurs"
  Then these "tx_requests":
  | nvid | created | status       | amount | payer | payee | for  | flags        | recursId |*
  |    1 | %now    | %TX_APPROVED |    200 | .ZZA  | .ZZB  | pmt  | self,funding |        8 |
  And count "tx_requests" is 1
  And these "txs2":
  | txid | amount | payee | completed | deposit |*
  |    1 |    200 | .ZZA  |         0 |       0 |
  And count "txs" is 2
  And count "txs2" is 1
  And count "tx_requests" is 1

  When cron runs "recurs"
  And cron runs "getFunds"
  Then count "txs" is 2
  And these "txs2":
  | txid | amount | payee | completed | deposit |*
  |    1 |    200 | .ZZA  |         0 |       0 |
  And count "txs2" is 1
  And count "tx_requests" is 1

# Rule: recurring payments to a member end when account is closed, but transactions from persist

Scenario: A recurring transaction cannot be completed because target account is closed
  Given members have:
  | uid  | flags |*
  | .ZZB |       |
  And these "tx_timed":
  | id | start   | from | to   | amount | period | purpose | end |*
  |  8 | %now-4m | .ZZA | .ZZB |    200 | week   | pmt     |     |
  |  9 | %now-3m | .ZZB | .ZZC |    200 | week   | pmt     |     |
  | 10 | %now-3d | .ZZC | .ZZB |    200 | week   | pmt     |     |
  When cron runs "recurs"
  Then count "txs" is 1
  And these "tx_timed":
  | id | end  |*
  |  8 | %now |
  |  9 |      |
  | 10 | %now |

Skip because member should be allowed to be invoiced?
Scenario: A recurring payment invoice cannot be completed because member is uncarded
  Given these "tx_requests":
  | nvid | created | status       | amount | payer | payee | for | flags  |*
  |    1 | %now    | %TX_APPROVED |     50 | .ZZA  | .ZZB  | pmt |        |
  And member ".ZZA" has no photo ID recorded
  When cron runs "getFunds"
  Then count "txs" is 1
  And count "tx_requests" is 1
Resume
