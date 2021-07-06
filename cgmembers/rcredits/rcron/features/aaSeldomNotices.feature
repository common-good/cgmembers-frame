Feature: Seldom Notices (this test fails when run after others)
AS a member
I WANT to hear about what's going on in my account
SO I can take appropriate action

Setup:
  Given members:
  | uid  | fullName   | flags     | email | notices      |*
  | .ZZA | Abe One    | ok        | a@    | %NOTICE_DFTS |
  | .ZZB | Bea Two    | member,ok | b@    | offer:d,need:d,tip:w,in:m,out:d,misc:w |
  | .ZZC | Corner Pub | co,ok     | c@    | %NOTICE_DFTS |
  And community email for member ".ZZA" is "%whatever@commongood.earth"

Scenario: a weekly notice member doesn't get notices on other days
  Given notices:
  | uid  | created | sent | message    | type |*
  | .ZZB | %today  |    0 | You stone. | misc |
  And it's time for ""
  When cron runs "notices"
  Then not these "notices":
  | uid  | created | sent   | message    |*
  | .ZZB | %today  | %today | You stone. |
  
# This test (and the next) fails near the end of February in or after a leap year (yearAgo+7d <> now+7d-1y)
Scenario: It's time to warn about an upcoming annual donation to CG
  Given members:
  | uid  | fullName | flags  | bankAccount        | activated   |*
  | .ZZD | Dee Four | ok     | USkk98765432100004 | %now-1y     |
  | .ZZE | Eve Five | ok     | USkk98765432100005 | %yearAgo+7d |
  And these "tx_timed":
  | id | action | start       | from | to  | amount | period | purpose |*
  |  1 | pay    | %yearAgo+7d | .ZZD | cgf |      1 | year   | gift!   |
  And transactions:
  | xid | created     | amount | payer | payee | purpose | flags       | recursId |*
  |   1 | %yearAgo+7d | 10     | .ZZD  | cgf   | gift!   | gift,recurs | 1        |
  When cron runs "annualGift"
  Then we email "annual-gift" to member "d@example.com" with subs:
  | amount | when    | atag | track |*
  |     $1 | %mdY+7d |    ? |     ? |
  And we email "annual-gift" to member "e@example.com" with subs:
  | amount | when    | atag | track |*
  |     $0 | %mdY+7d |    ? |     ? |

Scenario: It's time to renew backing
  Given members:
  | uid  | fullName | flags  | backing | backingDate | backingNext |*
  | .ZZD | Dee Four | ok     |       4 | %yearAgo+7d | %NUL        |
  | .ZZE | Eve Five | ok     |       5 | %yearAgo+7d | 3           |
  | .ZZF | Fox Co   | ok,co  |       6 | %yearAgo+8d | %NUL        |
  | .ZZG | Glo Sevn | ok     |       7 | %yearAgo+6d | %NUL        |
  | .ZZH | Hal Co   | ok,co  |       8 | %yearAgo+7d | %NUL        |
  | .ZZI | Ivy Nine | ok     |       9 | %yearAgo-1d | %NUL        |
  | .ZZJ | Joe Ten  | ok     |      10 | %yearAgo-1d | 4           |
  When cron runs "renewBacking"
  Then we email "renew-backing" to member "d@example.com" with subs:
  | amount | when    | atag | track |*
  |     $4 | %mdY+7d |    ? |     ? |
  And we email "renew-backing" to member "h@example.com" with subs:
  | amount | when    | atag | track |*
  |     $8 | %mdY+7d |    ? |     ? |
  And we do not email "renew-backing" to member "e@example.com"
  And we do not email "renew-backing" to member "f@example.com"
  And we do not email "renew-backing" to member "g@example.com"
  And we do not email "renew-backing" to member "i@example.com"
  And we do not email "renew-backing" to member "j@example.com"
  And members:
  | uid  | backing | backingDate | backingNext |*
  | .ZZD |       4 | %yearAgo+7d | %NUL        |
  | .ZZE |       5 | %yearAgo+7d | 3           |
  | .ZZF |       6 | %yearAgo+8d | %NUL        |
  | .ZZG |       7 | %yearAgo+6d | %NUL        |
  | .ZZH |       8 | %yearAgo+7d | %NUL        |
  | .ZZI |       9 | %daystart   | %NUL        |
  | .ZZJ |       4 | %daystart   | %NUL        |
