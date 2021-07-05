Feature: FBO
AS a sponsored member company or administrator
I WANT to track payments to and from non-members
SO I can accept donations and make payments for a fiscally-sponsored organization.

Setup:
  Given members:
  | uid  | fullName | address | city  | state  | zip | country  | postalAddr | floor | flags              | coFlags   |*
  | .ZZA | Abe One  | 1 A St. | Atown | Alaska | 01000 | US     | 1 A, A, AK |  -250 | ok,confirmed,debt  |           |
  | .ZZB | Bea Two  | 2 B St. | Btown | Utah   | 02000 | US     | 2 B, B, UT |     0 | ok,confirmed       |           |
  | .ZZC | Our Pub  | 3 C St. | Ctown | Cher   |       | France | 3 C, C, FR |     0 | ok,confirmed,co    | sponsored |
  And relations:
  | main | agent | permission |*
  | .ZZA | .ZZB  | buy        |
  | .ZZB | .ZZA  | read       |
  | .ZZC | .ZZB  | buy        |
  | .ZZC | .ZZA  | manage     |
  Then balances:
  | uid  | balance |*
  | .ZZA |       0 |
  | .ZZB |       0 |
  | .ZZC |       0 |

Scenario: A non-member donates to a sponsored member
  When member "C:A" visits "tx/charge"
  Then we show "Charge" with:
  | Member      | Non-member |
  | Full Name   | |
  | Postal Addr | |
  | City        | |
  | State       | |
  | Zip         | |
  | Amount      | |
  | For         | |
  | Category    | |
  
  When member "C:A" submits "tx/charge" with:
  | op     | fbo | fullName | address | city | state | zip   | amount | purpose | cat |*
  | charge | 1   | Dee Forn | 4 Fr St | Fton | MA    | 01004 | 100    | grant   |   2 |
  Then we scrip "tx" with subs:
  | field | question            | selfErr | payDesc | chargeDesc |*
  | who   | %_%amount to %name? | self-tx | Pay     | Charge     |
  # choice between Pay and Charge gets set in JS
  And we say "status": "info saved"
  And these "txs":
  | xid | payer      | payee | amount | purpose | cat | type     |*
  | 1   | %OUTER_UID | .ZZC  | 100    | grant   | 2   | %E_OUTER |
  And these "txs2":
  | xid | payee | amount | completed | deposit | pid |*
  | 1   | .ZZC  | 100    | %now      |       0 | 1   |
  And these "people":
  | pid | fullName | address | city | state | zip   |*
  | 1   | Dee Forn | 4 Fr St | Fton | MA    | 01004 |
  And balances:
  | uid  | balance |*
  | .ZZA |       0 |
  | .ZZB |       0 |
  | .ZZC |     100 |

Scenario: A non-member pays a sponsored member
  When member "C:A" visits "tx/pay"
  Then we show "Pay" with:
  | Member      | Non-member |
  | Full Name   | |
  | Postal Addr | |
  | City        | |
  | State       | |
  | Zip         | |
  | Amount      | |
  | For         | |
  | Category    | |
  
  When member "C:A" submits "tx/pay" with:
  | op  | fbo | fullName | address | city | state | zip   | amount | purpose  | cat |*
  | pay | 1   | Dee Forn | 4 Fr St | Fton | MA    | 01004 | 100    | printing |   3 |
  Then we scrip "tx" with subs:
  | field | question            | selfErr | payDesc | chargeDesc |*
  | who   | %_%amount to %name? | self-tx | Pay     | Charge     |
  # choice between Pay and Charge gets set in JS
  And we say "status": "info saved"
  And these "txs":
  | xid | payer      | payee | amount | purpose  | cat | type     |*
  | 1   | %OUTER_UID | .ZZC  | -100   | printing | 3   | %E_OUTER |
  And these "txs2":
  | xid | payee | amount | completed | deposit | pid |*
  | 1   | .ZZC  | -100   | %now      |       0 | 1   |
  And these "people":
  | pid | fullName | address | city | state | zip   |*
  | 1   | Dee Forn | 4 Fr St | Fton | MA    | 01004 |
  And balances:
  | uid  | balance |*
  | .ZZA |       0 |
  | .ZZB |       0 |
  | .ZZC |    -100 |

Scenario: A sponsored member views their transaction history
  Given these "txs":
  | xid | payer      | payee | amount | purpose  | cat | type     | payeeAgent |*
  | 1   | %OUTER_UID | .ZZC  | 100    | grant    | 2   | %E_OUTER | .ZZB       |
  | 2   | %OUTER_UID | .ZZC  | -200   | printing | 21  | %E_OUTER | .ZZB       |
  And these "txs2":
  | xid | payee | amount | completed | deposit | pid |*
  | 1   | .ZZC  | 100    | %now      |       0 | 4   |
  | 2   | .ZZC  | -100   | %now      |       0 | 5   |
  And these "people":
  | pid | fullName | address | city | state | zip   |*
  | 4   | Dee Forn | 4 D St  | Dton | MA    | 01004 |
  | 5   | Eva Fivn | 5 E St  | Eton | CA    | 01005 |
  When member "C:A" visits "history/transactions"
  Then we show "Transaction History" with:
  | Tx | Date | Name                  | Purpose  | Amount  | Balance |
  | 2  | %mdy | Eva Fivn (non-member) | printing | -200.00 | -100.00 |
  | 1  | %mdy | Dee Forn (non-member) | grant    |  100.00 |  100.00 |

  When member "C:A" visits "history/transaction/xid=1"
  Then we show "Transaction #1 Detail" with:
  | Date        | %mdY |
  | Amount      | $100 |
  | From        | Dee Forn (non-member) |
  | Postal Addr | 4 D St, Dton, MA 01004 |
  | For         | grant |
  | Category    | Government grants |
  | To Agent    | Bea Two |
  | Channel     | Web |
  