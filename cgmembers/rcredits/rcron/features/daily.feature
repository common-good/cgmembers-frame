Feature: Daily
AS a member
I WANT appropriate actions to be taken with respect to my account
SO things will be as they should be.

Setup:
  Given members:
  | uid  | fullName | minimum | savingsAdd | saveWeekly | achMin | floor | risks   | flags                 |*
  | .ZZA | Abe One  |    -100 |          0 |         20 |     20 |    10 | hasBank | ok,confirmed,refill   |
  | .ZZB | Bea Two  |     100 |          0 |         20 |     20 |    10 | hasBank | ok,confirmed,cashoutW |

#Scenario: A trial company runs out of time
#  Given members:
#  | uid       | .AAA           |**
#  | fullName  | Coco Co        |
#  | email     | a@             |
#  | flags     | confirmed co depends ok |
#  | activate  | %now-15d       |
#  When cron runs "everyDay"
#  Then we message "trial co end" to member ".AAA"
#  And members have:
#  | uid  | flags        | task |*
#  | .AAA | confirmed co | co2  |
