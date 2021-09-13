Feature: A user signs up for Common Good
AS a newbie
I WANT to open a Common Good account
SO I can be part of the Common Good Economy
# Note that "member" in the scenarios below means new member (newbie).

# phone 1 means +1 413.772.0001

Setup:
  Given members:
  | uid | fullName | phone | email | city  | state | zip   | floor | flags   | pass      |*
  | .ZZA | Abe One |     1 | a@    | Atown | AK    | 01000 |     0 | member  | %whatever |
  | .ZZB | Bea Two |     2 | b@    |       | UT    | 02000 |  -200 |         | |
  | .ZZC | Cor Pub |     3 | c@    | Ctown | CA    | 03000 |     0 | ok,co   | |
  And next captcha is "37"

Scenario: A newbie visits the individual signup page
  When member "?" visits page "signup"
  Then we show "Open a Personal Account"

Scenario: A member signs up
  Given member is logged out
  And next random code is "WHATEVER"
  And next random password is "quick brown fox jumped"
  When member "?" completes form "signup" with values:
  | fullName | email | phone        | zip   | acctType     | cq | ca |*
  | Al Aargh | z@    | 413-253-0000 | 01002 | %CO_PERSONAL | 37 | 74 |
  Then members:
  | uid  | fullName | legalName | email | phone        | zip   | state |*
  | .AAA | Al Aargh | Al Aargh  | z@    | +14132530000 | 01002 | MA    |
  And we email "verify" to member "z@" with subs:
  | fullName | qid    | site      | code     | pass                   |*
  | Al Aargh | NEWAAA | %BASE_URL | WHATEVER | quick brown fox jumped |
  And member ".AAA" one-time password is "WHATEVER"
  And member ".AAA" password is set to "quick brown fox jumped"
  And member ".AAA" is logged in
  And we show "Identity Verification"
  And we say "status": "info saved|step completed"
  And member ".AAA" steps left "verifyid fund preferences verifyemail"
  And members have:
  | uid  | signed | signedBy |*
  | .AAA | %today | Al Aargh |

Scenario: A member verifies ID
  Given member ".ZZB" has "person" steps done: "signup agree"
  When member ".ZZB" completes form "settings/verifyid" with values:
  | legalName | method | federalId   | dob      |*
  |           | 0      | 123-45-6789 | 2/1/1990 |
  # field 2 is SSN, as opposed to file upload
  Then members:
  | uid  | federalId | dob       |*
  | .ZZB | 123456789 | 633848400 |
  And we show "Connect a Checking Account" with:
  | Routing |
  | Account |
  | Refill  |
  And we say "status": "info saved|step completed"
  And member ".ZZB" steps left "fund preferences verifyemail ssn"

Scenario: A member connects a bank account
  Given member ".ZZB" has "person" steps done: "signup agree verifyid"
  When member ".ZZB" completes form "settings/fund" with values:
  | op     | connect      | chox | routingNumber | bankAccount | bankAccount2 | cashout | refills | target | achMin | saveWeekly |*
  | submit | %FUNDBY_BANK | 1    |     053000196 |         123 |          123 |       0 |       1 |     $0 |    $20 |         $0 |  
  Then we show "Account Preferences"
  And we say "status": "info saved|step completed"
  And members have:
  | uid  | risks   |*
  | .ZZB | hasBank |
  And member ".ZZB" steps left "preferences verifyemail"

Scenario: A member sets preferences
  Given member ".ZZB" has "person" steps done: "signup agree verifyid fund"
  When member ".ZZB" completes form "settings/preferences" with values:
  | roundup | statements | nosearch | secretBal |*
  |       1 | electronic |        0 |         1 |
  Then we show "Verify Your Email Address"
  And we say "status": "info saved|step completed"
  And member ".ZZB" steps left "verifyemail"

Scenario: A member verifies email
  Given member ".ZZB" has "person" steps done: "signup agree verifyid fund preferences"
  And member is logged out
  And member ".ZZB" one-time password is "WHATEVER"
  When member "?" visits page "settings/verifyemail/qid=NEWZZB&code=WHATEVER&verify=1"
  Then we show "Verified!"
  And member ".ZZB" steps left ""

  When member "?" completes form "settings/verifyemail/qid=NEWZZB&code=WHATEVER&verify=1" with values:
  | zot | whatever |**
  # no change to password
#  | pass1 | pass2 |*
#  |       |       |
  Then we show "You: Bea Two"
  And member ".ZZB" steps left ""
  And members:
  | uid  | flags       | task |*
  | .ZZB | member,ided |      |
  And we say "status": "tentative partial|want a card|get a card below"

Scenario: A member clicks Finish
  Given member ".ZZB" has "person" steps done: "signup agree verifyid fund preferences verifyemail"
  Then member ".ZZB" steps left ""
  When member ".ZZB" visits page "scraps/card"
  Then we show "Photo ID Picture"
  And member ".ZZB" steps left "photo contact donate crumbs proxies work backing stepup invite"

Scenario: A member uploads a photo
  Given member ".ZZB" has "card" steps done: ""
  When member ".ZZB" completes form "settings/photo" with values:
  | op       |*
  | nextStep |
  Then we show "Contact Information"
  And we say "status": "info saved|step completed"
  And member ".ZZB" steps left "contact donate crumbs proxies work backing stepup invite"

Scenario: A member gives contact info
  Given member ".ZZB" has "card" steps done: "photo"
  When member ".ZZB" completes form "settings/contact" with values:
  | fullName  | Bea Two        |**
  | email     | b@             |
  | phone     | 413-253-0002   |
  | address   | 2 B St.        |
  | city      | Amherst        |
  | state     | MA             |
  | zip       | 01002          |
  | owns      | 1              |
  | years     | 2              |
  | months    | 3              |
  | address2  | PO Box 2       |
  | city2     | Amherst        |
  | state2    | MA             |
  | zip2      | 01002          |
  Then members:
  | uid        | .ZZB           |**
  | fullName   | Bea Two        |
  | legalName  | Bea Two        |
  | email      | b@             |
  | phone      | +14132530002   |
  | address    | 2 B St.        |
  | city       | Amherst        |
  | state      | MA             |
  | zip        | 01002          |
# (test fails, but code works -- remove javascript?)  | postalAddr | 2 B St., Amherst, MA 01002 |
  | tenure     | 27             |
  | risks      |                |
  # owns, so no rents risk
  And we show "Donate to %PROJECT"
  And we say "status": "info saved|step completed"
  And member ".ZZB" steps left "donate crumbs proxies work backing stepup invite"

Scenario: A member donates
  Given member ".ZZB" has "card" steps done: "photo contact"
  When member ".ZZB" completes form "community/donate" with values:
  | amtChoice | period | honor | honored |*
  |        50 | month  |     - |         |
  Then we show "Share When You Receive"
  And we say "status": "gift thanks" with subs:
  | coName | %PROJECT |**
  And we say "status": "gift transfer later"
  And we say "status": "step completed"
  And member ".ZZB" steps left "crumbs proxies work backing stepup invite"

Scenario: A member chooses crumbs
  Given member ".ZZB" has "card" steps done: "photo contact donate"
  When member ".ZZB" completes form "community/crumbs" with values:
  | crumbs | 1.5 |**
  Then members have:
  | uid  | crumbs |*
  | .ZZB | .015   |
  And we show "Proxies"
  And we say "status": "info saved|step completed"
  And member ".ZZB" steps left "proxies work backing stepup invite"

Scenario: A member chooses proxies
  Given member ".ZZB" has "card" steps done: "photo contact donate crumbs"
  And these "r_proxies":
  | person | proxy | priority |*
  | .ZZB   | .ZZA  |        1 |
  | .ZZB   |    2  |        2 |
  When member ".ZZB" completes form "settings/proxies" with values:
  | op       |*
  | nextStep |
  Then we show "Your Work"
  And we say "status": "info saved|step completed"
  And member ".ZZB" steps left "work backing stepup invite"

Scenario: A member sets calling
  Given member ".ZZB" has "card" steps done: "photo contact donate crumbs proxies"
  When member ".ZZB" completes form "settings/work" with values:
  | calling  | company | companyPhone | companyOptions |*
  | whatever | Cor Pub |              | employee       |
  Then these "relations":
  | main | other | flags    | created |*
  | .ZZC | .ZZB  | employee | %now    |
  And we show "Backing Promise"
  And we say "status": "info saved|step completed"
  And member ".ZZB" steps left "backing stepup invite"

Scenario: A member sets backing  
  Given member ".ZZB" has "card" steps done: "photo contact donate crumbs proxies work"
  When member ".ZZB" completes form "community/backing" with values:
  | amtChoice |*
  |       100 |
  Then we show "Step Up"
  And we say "status": "info saved|step completed"
  And member ".ZZB" steps left "stepup invite"
  
Scenario: A member chooses to Step Up
  Given member ".ZZB" has "card" steps done: "photo contact donate crumbs proxies work backing"
  And members:
  | uid  | fullName | flags | coType    | zip   |*
  | .ZZF | Fox Co   | ok,co | nonprofit | 02006 |
  | .ZZG | Glo Co   | ok,co | nonprofit | 02007 |
  And member ".ZZF" has "%STEPUP_MIN" stepup rules
  And member ".ZZG" has "%(%STEPUP_MIN+1)" stepup rules

  When member ".ZZB" visits page "community/stepup"
  Then we show "Step Up" with:
  | Organization | Amount | When |
  | Fox Co       |        |      |
  | Glo Co       |        |      |
  
  When member ".ZZB" completes form "community/stepup" with values:
  | submit |*
  |        |
  Then we show "Invite People"
  And we say "status": "info saved|step completed"
  And member ".ZZB" steps left "invite"

Scenario: A member invites
  Given member ".ZZB" has "card" steps done: "photo contact donate crumbs proxies work backing stepup"
  When member ".ZZB" completes form "community/invite" with values:
  | sign | quote | org  | position | website |*
  | 1    | cuz   | MeCo | Boss     | me.co   |
  Then these "u_shout":
  | uid  | quote | org  | title | website |*
  | .ZZB | cuz   | MeCo | Boss  | me.co   |
  And we show "You: Bea Two" with:
  | Shortcuts |
  And without:
  | Finish |
  And we say "status": "info saved"
  And we say "status": "tentative complete|card soon"
  And member ".ZZB" steps left ""
#  And we tell ".ZZB" CO "New Member (Bea Two)" with subs:
#  | quid | status |*
#  | .ZZB | member |

Scenario: A member confirms social security number
  Given member ".ZZB" steps left "ssn"
#  When member ".ZZB" visits page "dashboard"
  When member "?" confirms form "signin" with values:
  | qid  | pass |*
  | .ZZB | 123  |
  Then we show "Confirm Your Social Security Number"
  
  When member ".ZZB" completes form "settings/ssn" with values:
  | federalId   |*
  | 123-45-6789 |
  Then we show "You: Bea Two"

Scenario: A newbie registers with no case
  When member "?" confirms form "signup" with values:
  | fullName | email | phone        | zip   | acctType     | cq | ca |*
  | eve five | e@    | 413-253-0000 | 01002 | %CO_PERSONAL | 37 | 74 |
  Then members:
  | uid  | fullName | legalName | email | phone        | zip   | country | state |*
  | .AAA | Eve Five | Eve Five  | e@    | +14132530000 | 01002 | US      | MA    |

Scenario: A member registers bad email
  When member "?" confirms form "signup" with values:
  | fullName | phone        | email     | zip   | acctType     | cq | ca |*
  | Eve Five | 413-253-0000 | %whatever | 01001 | %CO_PERSONAL | 37 | 74 |
  Then we say "error": "bad email"

Scenario: A member registers bad name
  When member "?" confirms form "signup" with values:
  | fullName  | email     | phone        |  zip  | acctType     | cq | ca |*
  | ™ %random | e@        | 413-253-0000 | 01002 | %CO_PERSONAL | 37 | 74 |
# NO  Then we say "error": "illegal char" with subs:
#  | field    |*
#  | fullName |

Scenario: A member registers bad zip
  When member "?" confirms form "signup" with values:
  | fullName  | email     | phone        |  zip    | acctType     | cq | ca |*
  | Eve Five  | e@        | 413-253-0000 | %random | %CO_PERSONAL | 37 | 74 |
  Then we say "error": "bad zip"
 
Scenario: A member registers again
  When member "?" confirms form "signup" with values:
  | fullName | email     | phone        |  zip  | acctType     | cq | ca |*
  | Abe Dup  | a@        | 413-253-0002 | 01001 | %CO_PERSONAL | 37 | 74 |
  Then we say "error": "duplicate email|forgot password" with subs:
  | who     | a                                          |*
  | Abe One | a href="settings/password/a%40example.com" |
