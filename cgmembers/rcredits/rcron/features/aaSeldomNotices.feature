Feature: Seldom Notices (this test fails when run after others)
AS a member
I WANT to hear about what's going on in my account
SO I can take appropriate action

Setup:
  Given members:
  | uid  | fullName   | flags     | email |*
  | .ZZA | Abe One    | ok        | a@    |
  | .ZZB | Bea Two    | member,ok,weekly | b@    |
  | .ZZC | Corner Pub | co,ok     | c@    |
  And community email for member ".ZZA" is "%whatever@rCredits.org"

Scenario: a weekly notice member doesn't get notices on other days
  Given notices:
  | uid  | created | sent | message    |*
  | .ZZB | %today  |    0 | You stone. |
  And it's time for ""
  When cron runs "notices"
  Then not these "notices":
  | uid  | created | sent   | message    |*
  | .ZZB | %today  | %today | You stone. |