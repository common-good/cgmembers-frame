Feature: Password strength
AS a member
I WANT to be kept from choosing a password that is easy to guess
SO my account is protected even if someone skips the strength bar in the browser

# Rule: every member meets the same minimum the strength bar shows; common words, name and email count as one letter each

Scenario: The server applies the same minimum as the strength bar
  Then these passwords meet the member minimum:
  | pw                | avoid  | ok  |*
  | k                 |        | no  |
  | password          |        | no  |
  | Passw0rd          |        | no  |
  | Passw0rd!         |        | no  |
  | Abcd1234          |        | no  |
  | aB3$efgh          |        | yes |
  | aB3$efg           |        | no  |
  | abcdefghijklmno1  |        | no  |
  | abcdefghijklmnoP1 |        | yes |
  | Zephyr9$Ab        |        | yes |
  | Zephyr9$Ab        | zephyr | no  |
  | zephyr9$AbCdEf    | zephyr | yes |
