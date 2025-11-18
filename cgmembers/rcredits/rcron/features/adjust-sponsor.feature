Feature: Adjust Sponsor
AS an organization fiscally sponsored by Common Good
I WANT my fiscal sponsorship fee to be adjusted automatically when appropriate
SO I/we pay the amount required by our contract.

Setup:
  Given members:
  | uid  | fullName | phone        | address | city  | state  | zip | country  | postalAddr | floor | flags              | coFlags   | emailCode |*
  | .ZZA | Abe One  | +13013013001 | 1 A St. | Atown | Alaska | 01000 | US     | 1 A, A, AK |  -250 | ok,confirmed,debt  |           |           |
  | .ZZB | Bea Two  |              | 2 B St. | Btown | Utah   | 02000 | US     | 2 B, B, UT |  -250 | ok,confirmed,debt  |           |           |
  | .ZZC | Our Pub  | +13333333333 | 3 C St. | Ctown | Cher   |       | France | 3 C, C, FR |     0 | ok,confirmed,co    | sponsored | Cc3       |
  And these "people":
  | pid | fullName | address | city | state | zip   |*
  | 4   | Dee Forn | 4 D St  | Dton | MA    | 01004 |
  | 5   | Eva Fivn | 5 E St  | Eton | CA    | 01005 |
  And these "u_relations":
  | main | agent | permission |*
  | .ZZA | .ZZB  | buy        |
  | .ZZB | .ZZA  | read       |
  | .ZZC | .ZZB  | buy        |
  | .ZZC | .ZZA  | manage     |
  And these "tx_rules":
  | id        | 1            |**
  | payer     |              |
  | payerType | %REF_ANYBODY |
  | payee     | .ZZC         |
  | payeeType | %REF_ACCOUNT |
  | from      | %MATCH_PAYEE |
  | to        | cgf          |
  | action    | %ACT_SURTX   |
  | amount    | 0            |
  | portion   | .05          |
  | purpose   | %FS_NOTE (5%)|
  | minimum   | 0            |
  | useMax    |              |
  | amtMax    |              |
  | template  |              |
  | start     | %now         |
  | end       |              |
  | code      |              |
  And these "txs2":
  | xid | payee | amount | completed | deposit | pid |*
  | 1   | .ZZC  | 250000 | %now      |       0 | 4   |
  | 2   | .ZZC  | -100   | %now      |       0 | 5   |
  And these "txs":
  | xid | payer      | payee | amount | purpose | cat2        | type     | agt2 | flags |*
  | 1   | %UID_OUTER | .ZZC  | 250000 | grant   | D-FBO       | %E_OUTER | .ZZB | gift  |
  | 2   | %UID_OUTER | .ZZC  | -200   | labor   | FBO-LABOR   | %E_OUTER | .ZZB |       |
  And these "tx_entries":
  | id | xid | uid  | amount | description   | entryType | cat    | rule |*
  | -4 | 1   | .ZZC | -12500 | %FS_NOTE (5%) | %E_AUX    | D-FBO  | 1    |
  | 4  | 1   | cgf  | 12500  | %FS_NOTE (5%) | %E_AUX    | FS-FEE | 1    |
  Then balances:
  | uid  | balance |*
  | .ZZC | 237300  |
  | cgf  | 12500   |

Scenario: A sponsored member views their transaction history
  When cron runs "adjustFS"
  Then these "tx_rules":
  | id | portion | purpose       |*
  | 1  | .04     | %FS_NOTE (4%) |
  
  And these "txs":
  | xid | payer | payee | amount | for                               | cat2   | type     | agt2 | flags |*
  | 3   | .ZZC  | cgf   | -1000  | fiscal sponsorship fee adjustment | FS-FEE | %E_PRIME | cgf  |       |
