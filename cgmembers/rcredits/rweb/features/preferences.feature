Feature: Preferences
AS a member
I WANT to set certain preferences
SO I can automate and control the behavior of my rCredits account.

Setup:
  Given members:
  | uid  | crumbs | minimum | savingsAdd | saveWeekly | achMin | backing | floor | flags   |*
  | .ZZA |    .01 |     100 |          0 |          1 |     20 |     100 |    10 | ok,confirmed,nosearch,paper |
  | .ZZB |    .02 |     -10 |         10 |          0 |     50 |      10 |     0 | ok,co,confirmed,weekly,secret |
  
Scenario: A member visits the preferences page
  When member ".ZZA" visits page "settings/preferences"
  Then we show "Account Preferences" with:
  | SNAP Number |  |
  | Round Up?   |  |
  And radio "statements" is "printed statements"
  And radio "notices" is "daily"
  And we show checked:
  | No Search | by name or account ID only |
  And we show unchecked:
  | Secret Balance | Don't let merchants |
  And with:
  | Food Fund |  |
  | Backing   | $100 |

Scenario: Another member visits the preferences page
  When member ".ZZB" visits page "settings/preferences"
  Then we show "Account Preferences" with:
  | Crumbs | 2 |
  And radio "statements" is "accept electronic"
  And radio "notices" is "weekly"
  And we show checked:
  | Secret Balance | Don't let merchants |
  And with:
  | Food Fund |  |
  | Backing   | $10 |

Scenario: A member changes preferences
  Given transactions: 
  | xid | created   | amount | from | to   | purpose |*
  |   3 | %today-1m |    250 | ctty | .ZZA | grant   |
  And member ".ZZA" has no photo ID recorded
  When member ".ZZA" completes form "settings/preferences" with values:
  | roundup | crumbs | notices | statements | nosearch | secretBal | food | snap |*
  |       1 |      3 | monthly | electronic |        0 |         1 |    5 | 04-293-38-A2837 |
  Then members:
  | uid  | crumbs | food | snap         | flags   |*
  | .ZZA |    .03 | 0.05 | 0429338A2837 | ok,member,confirmed,monthly,secret,roundup |
