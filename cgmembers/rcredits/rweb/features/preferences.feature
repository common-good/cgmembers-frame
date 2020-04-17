Feature: Preferences
AS a member
I WANT to set certain preferences
SO I can automate and control the behavior of my rCredits account.

Setup:
  Given members:
  | uid  | crumbs | minimum | savingsAdd | saveWeekly | achMin | backing | floor | flags   |*
  | .ZZA |    .01 |     100 |          0 |          1 |     20 |     100 |    10 | ok,confirmed,nosearch,paper |
  | .ZZC |    .02 |     -10 |         10 |          0 |     50 |      10 |     0 | ok,co,confirmed,weekly,secret |
  And these "relations":
  | reid | main | other | permission |*
  |    1 | .ZZC | .ZZA  | manage     |
  
Scenario: A member visits the preferences page
  When member ".ZZA" visits page "settings/preferences"
  Then we show "Account Preferences" with:
  | Round Up         |  |
  | Food Assistance? |  |
  And radio "statements" is "printed statements"
  And radio "notices" is "daily"
  And radio "secretBal" is "No"
  And radio "nosearch" is "Yes"
  And with:
  | Backing   | $100 |
  And without:
  | Food Fund |  |

Scenario: A company agent visits the preferences page
  When member "C:A" visits page "settings/preferences"
  Then we show "Account Preferences" with:
  | Statements |     |
  | Backing    | $10 |
  And without:
  | Food Assistance? |  |  
  And radio "statements" is "accept electronic"
  And radio "notices" is "weekly"
  And radio "secretBal" is "Yes"

Scenario: A member changes preferences
  Given transactions: 
  | xid | created   | amount | payer | payee | purpose |*
  |   3 | %today-1m |    250 | ctty | .ZZA | grant   |
  And member ".ZZA" has no photo ID recorded
  When member ".ZZA" completes form "settings/preferences" with values:
  | roundup | notices | statements | nosearch | secretBal | snap |*
  |       1 | monthly | electronic |        0 |         1 |    1 |
  Then members:
  | uid  | snap | flags   |*
  | .ZZA |    1 | ok,member,confirmed,monthly,secret,roundup |
