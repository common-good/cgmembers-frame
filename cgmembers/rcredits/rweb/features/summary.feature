Feature: Summary
AS a member
I WANT to see an overview of an account
SO I know where it stands.

# use weeks, not months, for consistent test statistics

Setup:
  Given members:
  | uid  | fullName   | postalAddr                   | floor | flags      |*
  | .ZZA | Abe One    | 1 A St., Atown, AK 01000     | -100  | ok         |
  | .ZZB | Bea Two    | 2 B St., Btown, UT 02000     | -200  | ok,roundup |
  | .ZZC | Corner Pub | 3 C St., Ctown, Cher, FRANCE | -300  | ok,co      |
  And members have:
  | uid  | created   |*
  | ctty | %today-9w |
  | .ZZA | %today-7w |
  | .ZZB | %today-6w |
  | .ZZC | %today-6w |
  And usd transfers:
  | payee | amount | completed |*
  | .ZZA  |  100   | %today-7w |
  | .ZZB  |  200   | %today-6w |
  | .ZZC  |  300   | %today-6w |
  And relations:
  | main | agent | num | permission |*
  | .ZZA | .ZZB  |   1 | buy        |
  | .ZZB | .ZZA  |   1 | read       |
  | .ZZC | .ZZB  |   1 | buy        |
  | .ZZC | .ZZA  |   2 | sell       |
  And transactions: 
  | xid | created   | amount | payer | payee | purpose      |*
  |   1 | %today-7w |      0 | ctty | .ZZA | signup       |
  |   2 | %today-6w |      0 | ctty | .ZZB | signup       |
  |   3 | %today-6w |      0 | ctty | .ZZC | signup       |
  |   4 | %today-5w |     10 | .ZZB | .ZZA | cash E       |
  |   5 | %today-4w |     20 | .ZZC | .ZZA | usd F        |
  |   6 | %today-3w |     40 | .ZZA | .ZZB | whatever43   |
  |   7 | %today-2d |      5 | .ZZB | .ZZC | cash J       |
  |   8 | %today-1d |     80 | .ZZA | .ZZC | whatever54   |
  Then balances:
  | uid  | balance |*
  | .ZZA |      10 |
  | .ZZB |     225 |
  | .ZZC |     365 |
  Given cron runs "acctStats"

Scenario: A member clicks the summary tab
  When member ".ZZA" visits page "summary"
  Then we show "Account Summary" with:
  | ID        | ZZA |
  | ~...      | (personal account) |
  | Name      | Abe One (abeone) |
#  | Contact   | 1 A St., Atown, AK 01000 |
  | Balance   | $10 |
  
Scenario: A member clicks the summary tab with roundups
  Given transactions:
  | xid | created | amount | payer | payee | purpose |*
  |   9 | %today  |  80.02 | .ZZB | .ZZC | goodies |
  When member ".ZZB" visits page "summary"
  Then balances:
  | uid  | balance |*
  | .ZZB |  144.98 |
  And we show "Account Summary" with:
  | Name          | Bea Two (beatwo) |
  | Balance       | $144 |

Scenario: An agent clicks the summary tab without permission to manage
  When member "A:B" visits page "summary"
  Then we show "Account Summary" with:
  | ID        | ZZA |
  | ~...      | (personal account) |
  | Name | Abe One (abeone)   |
  And without:
  | Make This a Joint |

Scenario: A company agent clicks the summary tab
  When member "C:A" visits page "summary"
  Then we show "Account Summary" with:
  | ID       | ZZC |
  | ~...     | (company account) |
  | Name     | Corner Pub (cornerpub) |
#  | Contact  | 3 C St., Ctown, Cher, FRANCE |