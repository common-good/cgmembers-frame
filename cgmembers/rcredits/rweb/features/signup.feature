Feature: A user signs up for Common Good
AS a newbie
I WANT to open a Common Good account
SO I can be part of the Common Good Economy
# Note that "member" in the scenarios below means new member (newbie).

Setup:
  Given members:
  | uid  | fullName | acctType    | flags      | created  |*
  | .ZZZ | Zeta Zot | personal    | ok         | 99654321 |
  And member is logged out

Scenario: A newbie visits the individual signup page
  When member "?" visits page "signup"
  Then we show "Open a Personal Account"

Scenario: A newbie registers in Western Massachusetts
  Given member is logged out
  And next random code is "WHATEVER"
  And next random password is "quick brown fox jumped"
  When member "?" confirms form "signup" with values:
  | fullName | email | phone        | zip   | acctType     |*
  | Abe One  | a@    | 413-253-0000 | 01002 | %CO_PERSONAL |
  Then members:
  | uid  | fullName | legalName | email | phone     | zip   | state | country |*
  | .AAA | Abe One  | Abe One   | a@ | +14132530000 | 01002 |     0 | US      |
  And these "signup":
  | preid | source | created |*
  | ?     | ?      | %now    |
  And we email "verify" to member "a@" with subs:
  | fullName | name   | qid    | site      | code     | pass                   |*
  | Abe One  | abeone | NEWAAA | %BASE_URL | WHATEVER | quick brown fox jumped |
  And member ".AAA" one-time password is set to "WHATEVER"
  And member ".AAA" password is set to "quick brown fox jumped"
  And member ".AAA" is logged in
  And we show "Identity Verification"
  And we say "status": "info saved|step completed"
  And steps left "verifyid agree preferences fund verifyemail"

Scenario: A newbie registers with no case
  When member "?" confirms form "signup" with values:
  | fullName | email | phone        | zip   | acctType     |*
  | abe one  | a@    | 413-253-0000 | 01002 | %CO_PERSONAL |
  Then members:
  | uid  | fullName | legalName | email | phone     | zip   | country | state |*
  | .AAA | Abe One  | Abe One   | a@ | +14132530000 | 01002 | US      | 0     |

Scenario: A member registers bad email
  When member "?" confirms form "signup" with values:
  | fullName | phone        | email     | zip   | acctType     |*
  | Abe One  | 413-253-0000 | %whatever | 01001 | %CO_PERSONAL |
  Then we say "error": "bad email"

Scenario: A member registers bad name
  When member "?" confirms form "signup" with values:
  | fullName  | email     | phone        |  zip  | acctType     |*
  | ™ %random | a@        | 413-253-0000 | 01002 | %CO_PERSONAL |
  Then we say "error": "illegal char" with subs:
  | field    |*
  | fullName |

Scenario: A member registers bad zip
  When member "?" confirms form "signup" with values:
  | fullName  | email     | phone        |  zip    | acctType     |*
  | Abe One   | a@        | 413-253-0000 | %random | %CO_PERSONAL |
  Then we say "error": "bad zip"
 
Scenario: A member registers again
  Given members:
  | uid  | fullName   | phone  | email |*
  | .ZZA | Abe One    | +20001 | a@    |
  When member "?" confirms form "signup" with values:
  | fullName  | email     | phone        |  zip  | acctType     |*
  | Bea Two   | a@        | 413-253-0002 | 01001 | %CO_PERSONAL |
  Then we say "error": "duplicate email|forgot password" with subs:
  | who     | a                                          |*
  | Abe One | a href="settings/password/a%40example.com" |
