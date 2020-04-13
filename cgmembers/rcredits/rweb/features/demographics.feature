Feature: Risk
AS a agent for a Common Good company
I WANT to know as much as possible about my customers
SO I can serve them better and do more business with them and others

Setup:
  Given members:
  | uid  | fullName   | address | zip   | flags      | risks    | tenure | moves | city | dob      |*
  | .ZZA | Abe One    | 1 A St. | 01001 | ok         | adminOk  | 21     | 0     | Aton | %now-81y |
  | .ZZB | Bea Two    | 2 A St. | 01001 | ok         | rents    | 43     | 1     | Aton | %now-72y |
  | .ZZC | Corner Pub | 3 C St. | 01003 | ok,co      | cashCo   | 18     |       | Aton | %now-23y |
  | .ZZD | Dee Four   | 3 C St. | 01004 | ok         | hasBank  | 25     | 0     | Dton | %now-24y |
  | .ZZE | Eve Five   | 5 A St. | 01005 | ok         | shady    | 1      | 0     | Dton | %now-25y |
  | .ZZF | Flo Six    | 6 A St. | 01006 | ok,roundup |          | 32     | 0     | Fton | %now-26y |
  | .ZZG | Guy Seven  | 7 A St. | 01007 | ok         | addrOff  | 11     | 5     | Gton | %now-27y |
  | .ZZH | Hal Eight  | 8 A St. | 01008 | ok         | ssnOff   | 100    | 10    | Hton | %now-28y |
  | .ZZI | Ida Nine   | 9 A St. | 01009 | ok         | fishy    | 3      | 20    | Iton | %now-29y |
  | .ZZJ | Jay Ten    | A J St. | 01010 | ok,co      |          | 5      | 0     | Jton | %now-30y |
  And these "invites":
  | inviter | invitee | email |*
  | .ZZA    | .ZZD    | d2@   |
  | .ZZA    |    0    | e@    |
  | .ZZG    | .ZZH    | h2@   |
  | .ZZG    | .ZZI    | i@    |
  And proxies:
  | person | proxy | priority |*
  | .ZZA   | .ZZB  |        1 |
  | .ZZA   | .ZZD  |        2 |
  | .ZZB   | .ZZD  |        1 |
  | .ZZB   | .ZZA  |        2 |
  | .ZZD   | .ZZA  |        1 |
  | .ZZD   | .ZZB  |        2 |
  And relations:
  | main | agent | permission | employee | owner | draw |*
  | .ZZC | .ZZA  | scan       |        Y |     0 |    0 |
  | .ZZC | .ZZB  | manage     |          |       |      |
  | .ZZC | .ZZD  |            |          |       |      |
  And these "recurs":
  | payer | payee | amount | period |*
  | .ZZA  |   cgf |     10 |     Y |
  | .ZZB  |   cgf |      5 |     Y |
  | .ZZC  |   cgf |      1 |     Y |
  | .ZZD  |   cgf |      1 |     Q |
  | .ZZE  |   cgf |      1 |     M |
  | .ZZF  |   cgf |      1 |     Y |
  | .ZZG  |   cgf |      5 |     Y |
  | .ZZH  |   cgf |      5 |     Y |
  | .ZZI  |   cgf |      5 |     Y |
# share is irrelevant here as long as it is non-negative
  And transactions: 
  | xid | created   | amount | payer | payee | purpose | channel |*
  |   1 | %today-7m |    250 | .ZZG | .ZZC | car     | %TX_SYS |
  |   2 | %today-6m |    250 | .ZZH | .ZZC | boat    | %TX_SYS |
  |   3 | %today-6m |    250 | .ZZI | .ZZC | fish    | %TX_SYS |
  |   4 | %today-5m |     10 | .ZZC | .ZZA | cash E  | %TX_POS |
  |   5 | %today-1m |   1100 | .ZZA | .ZZC | pop in  | %TX_POS |
  |   6 | %today-3w |    240 | .ZZB | .ZZC | what G  | %TX_POS |
  |   7 | %today-2w |     50 | .ZZD | .ZZC | cash P  | %TX_POS |
  |   8 | %today-1w |    120 | .ZZD | .ZZC | offline | %TX_POS |
  |   9 | %today-6d |    100 | .ZZF | .ZZJ | cash V  | %TX_WEB |
  |  10 | %today-1d |    120 | .ZZA | .ZZJ | this    | %TX_POS |
  |  11 | %today-1d |    120 | .ZZB | .ZZJ | that    | %TX_POS |
  |  12 | %today-1d |     40 | .ZZJ | .ZZC | labor   | %TX_WEB |
  |  13 | %today-1d |     10 | .ZZA | cgf  | gift    | %TX_WEB |
  |  14 | %today-1d |     11 | .ZZB | cgf  | gift    | %TX_WEB |  
  And usd transfers:
  | txid | payee | amount | completed |*
  |    1 | .ZZA  |    400 | %today-2m |  
  |    2 | .ZZB  |    100 | %today-2m |  
  |    3 | .ZZC  |    300 | %today-2m |  
  |    4 | .ZZE  |    200 | %today    |  
  |    5 | .ZZF  |   -600 | %today    |  
  |    6 | .ZZC  |   -500 | %today    |

Scenario: A company agent runs the demographics query
  When member "C:B" visits page "history/company-reports/demographics"
  Then we show "Customer Demographics" with:
  | Customer Count: |     7 |
  | Median Age:     |    29 |
  | Median Tenure:  | 23 months at current location |
  | Owns vs. Rents: | 85.7% |
  
  | Top Cities: | |
  | City/Town     	| Count |
  | Aton	          |     2 |
  | Dton	          |     1 |
  | Gton          	|     1 |
  | Hton          	|     1 |
  | Iton           	|     1 |
  | Jton	          |     1 |
  
  | Also Shop At: | |
  | Company	        | Count |
  | Jay Ten	        |     2 |
