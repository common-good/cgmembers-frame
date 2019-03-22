Feature: Community Bits
AS a Common Good community administrator
I WANT to know when I should ask local members to get their balance up to zero
SO our Common Good community doesn't go bankrupt.

Setup:
  Given members:
  | uid  | fullName   | address | city  | state | zip   | country | postalAddr | flags        | created   |*
  | .ZZA | Abe One    | 1 A St. | Atown | AL    | 01000 | US      | 1 A, A, AK | ok,confirmed | %today-5m |
  And balances:
  | uid  | balance | floor |*
  | cgf  |       0 |     0 |
  | .ZZA |     100 |   -20 |

Scenario: Community bans spending below zero
  Given members have:
  | uid  | flags    |*
  | ctty | ok,up,co |
  And stats:
  | created    | ctty | usdIn | usdOut |*
  | %today-90d | ctty |   200 |    -80 |
  | %today-60d | ctty |   201 |    -90 |
  | %today-30d | ctty |   202 |   -100 |
  | %today     | ctty |   203 |   -110 |
  When cron runs "cttyStats"
  Then we tell "ctty" CO "credit ban on" with subs:
  | months |*
  |      3 |
