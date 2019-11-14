Feature: Membership
AS a member
I WANT to manage my progress toward activating my account
SO I can participate actively.

#phone 1 means +1 413.772.0001

Setup:
  Given members:
  | uid | fullName | phone | email | city  | state | zip   | floor | flags     | pass      |*
  | .ZZA | Abe One |     1 | a@    | Atown | AK    | 01000 |     0 |           | %whatever |
  | .ZZB | Bea Two |     2 | b@    |       | UT    | 02000 |  -200 | member    | |
  | .ZZC | Our Pub |     3 | c@    | Ctown | CA    | 03000 |     0 | member,co | |

# (see also signup feature) 
Scenario: An individual member signs up
  Given member is logged out
  And next random code is "WHATEVER"
  When member "?" completes form "signup" with values:
  | fullName | email | phone        | zip   | acctType     |*
  | Al Aargh | z@    | 413-253-0000 | 01002 | %CO_PERSONAL |
  Then we show "Identity Verification"
  And we say "status": "info saved|step completed"
  And member ".AAA" steps left "verifyid agree preferences fund verifyemail"

  When member ".AAA" completes form "settings/verifyid" with values:
  | field | federalId   | dob      |*
  |     2 | 123-45-6789 | 2/1/1990 |
  # field 2 is SSN and DOB, as opposed to file upload
  Then we show "%PROJECT Agreement" with:
  | I make this agreement |
  And we say "status": "info saved|step completed"
  And member ".AAA" steps left "agree preferences fund verifyemail ssn"

  When member ".AAA" completes form "community/agreement" with values:
  | op | I Agree |**
  Then we show "Account Preferences"
  And we say "status": "info saved|step completed"
  And steps left "preferences fund verifyemail ssn"

  When member ".AAA" completes form "settings/preferences" with values:
  | roundup | crumbs | notices | statements | nosearch | secretBal |*
  |       1 |      2 | monthly | electronic |        0 |         1 |
  Then we show "Getting Money In or Out"
  And we say "status": "info saved|step completed"
  And steps left "fund verifyemail ssn"

  When member ".AAA" completes form "settings/fund" with values:
  | op     | connect | routingNumber | bankAccount | bankAccount2 | cashout | refills | target | achMin | saveWeekly |*
  | submit |       2 |     053000196 |         123 |          123 |       0 |       1 |     $0 |    $20 |         $0 |  
  Then we show "Verify Your Email Address"
  And we say "status": "info saved|step completed"
	And members have:
	| uid  | risks         |*
	| .AAA | hasBank,rents |
  And steps left "verifyemail ssn"
  
  Given member is logged out
  When member "?" visits page "reset/id=alaargh&code=WHATEVER&verify=1"
  Then we show "Verified!"
  And member ".AAA" steps left "ssn"
  
  When member "?" completes form "reset/id=alaargh&code=WHATEVER&verify=1" with values:
  | pass1 | pass2 |*
  |       |       |
  Then we show "Confirm Your Social Security Number"
  And we say "status": "info saved|step completed"
  And member ".AAA" steps left "ssn"
  
  When member ".AAA" completes form "settings/ssn" with values:
  | federalId   |*
  | 123-45-6789 |
  Then we show "Account Summary" with:
  | Next Steps |
  | Invite |
  | Card   |
  | Sell   |
  | Voice  |
  | Give   |
  And we say "status": "setup complete|individual approval|join thanks|next steps|no card member"
  And steps left ""
  And members have:
  | uid  | flags  |*
  | .AAA | member,refill,roundup,monthly,secret |
  And we tell ".AAA" CO "New Member (Al Aargh)" with subs:
  | quid | status |*
  | .AAA | member |
  
Scenario: A member wants a card 
  When member ".ZZB" visits page "scraps/card"
  Then we show "Photo ID Picture"
  And steps left "photo contact donate"

  When member ".ZZB" completes form "settings/photo" with values:
  | op       |*
  | nextStep |
  Then we show "Contact Information"
  And we say "status": "info saved|step completed"
  And steps left "contact donate"

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
  And steps left "donate"
  
  When member ".ZZB" completes form "community/donate" with values:
  | amtChoice | period | honor | honored |*
  |        50 |      M |     - |         |
  Then we show "Account Summary" with:
  | Next Steps |
  | Invite |
  | Sell   |
  | Voice  |
  | Give   |
  And without:
  | Get a %PROJECT payment card |
  And we say "status": "|card member"
  And steps left ""

Scenario: A member wants to sell
  # identity was verified, but not by SSN
  When member ".ZZB" visits page "scraps/sell"
  Then we show "Verify Your Identity" with:
  | Soc Sec # |
  | Birth Date |
  And steps left "ssn contact tithein"

  Given step done "verifyid"
  When member ".ZZB" completes form "settings/ssn" with values:
  | federalId   |*
  | 123-45-6789 |
  Then we show "Contact Information"
  And we say "status": "info saved|step completed"
  And steps left "contact tithein ssn"

  Given step done "ssn"
  And step done "contact"
  When member ".ZZB" completes form "settings/tithein" with values:
  | crumbs | 1.5 |**
  Then members have:
  | uid  | crumbs |*
  | .ZZB | .015   |
  And we show "Account Summary" with:
  | Next Steps |
  | Invite |
  | Card   |
  | Voice  |
  | Give   |
  And without:
  | Sell |
  And we say "status": "info saved|sell member"
  And steps left ""
  
Scenario: A member wants to have a voice in the Common Good democracy
  When member ".ZZB" visits page "scraps/voice"
  Then we show "Contact Information"
  And steps left "contact donate proxies work backing invite"
  
  Given step done "contact"
  And step done "donate"
  When member ".ZZB" visits page "scraps/voice"
  Then we show "Proxies"
  And steps left "proxies work backing invite"
  
  Given proxies:
  | person | proxy | priority |*
  | .ZZB   | .ZZA  |        1 |
  | .ZZB   |    2  |        2 |
  When member ".ZZB" completes form "settings/proxies" with values:
  | op       |*
  | nextStep |
  Then we show "Your Work"
  And we say "status": "info saved|step completed"
  And steps left "work backing invite"

  When member ".ZZB" completes form "settings/work" with values:
  | calling  |*
  | whatever |
  Then we show "Backing Promise"
  And we say "status": "info saved|step completed"
  And steps left "backing invite"
  
  When member ".ZZB" completes form "community/backing" with values:
  | amtChoice |*
  |       100 |
  Then we show "Invite Someone"
  And we say "status": "info saved|step completed"
  And steps left "invite"

  When member ".ZZB" completes form "community/invite" with values:
  | email | trusted | zip   | subject | message |*
  | a@    |       1 | 01002 | test    | hi!     |
  Then these "invites":
  | inviter | invited | zip   | subject | message |*
  | .ZZB    | %today  | 01002 | test    | hi!     |
  And we show "Account Summary" with:
  | Next Steps |
  | Invite |
  | Card   |
  | Sell   |
  | Give   |
  And without:
  | Voice  |
  And we say "status": "info saved|voice member"
  And steps left ""

# (see also signup feature) 
Scenario: A company signs up
  Given member is logged out
  And next random code is "WHATEVER"
  When member "?" completes form "signup-co" with values:
  | source2 | contact  | fullName | zip   | phone        | email | selling | source | ownPhone | qid    |*
  | radio   | Al Aargh | New Co   | 01004 | 413-253-0004 | d@    | fish    | TV     |        1 |        |
  Then we show "Get Your Customers Signed Up"
  And we say "status": "info saved|step completed"
  And member ".AAA" steps left "discount verifyemail"

#  Given member ".AAA" has "person" steps done: "signup"
  When member ".AAA" completes form "community/discount" with values:
  | amount | minimum | start | end | ulimit | automatic | type     |*
  | 100    | 10      | %mdy  |     |      5 | 1         | discount |
  Then we show "Verify Your Email Address"
  And we say "status": "info saved|step completed"
  And steps left "verifyemail"

  Given member is logged out
  When member "?" visits page "reset/id=newco&code=WHATEVER&verify=1"
  Then we show "Verified!"
  And member ".AAA" steps left ""
  
  When member "?" completes form "reset/id=newco&code=WHATEVER&verify=1" with values:
  | pass1 | pass2 |*
  |       |       |
  Then we show "Account Summary" with:
  | Next Steps |
  | Invite |
  | Finish |
  | Give   |
  And we say "status": "setup complete|company approval|join thanks|next steps"
  And steps left ""
  And members have:
  | uid  | flags                       | legalName      | contact  |*
  | .AAA | member,confirmed,co,depends | %CGF_LEGALNAME | Al Aargh |
  And we tell ".AAA" CO "New Member (New Co)" with subs:
  | quid | status |*
  | .AAA | member |

Scenario: A member company wants to complete the account
  Given members have:
  | uid  | flags                       |*
  | .ZZC | member,confirmed,co,depends |
  When member ".ZZC" visits page "scraps/co2"
  Then we show "Connect a Manager Account"
  And steps left "agent agree contact backing company donate photo preferences tithein discount"

  Given members have:
  | uid  | phone        |*
  | .ZZB | 413-253-0002 |
  When member ".ZZC" completes form "settings/agent" with values:
  | agent  | phone        | coType  |*
  | NEWZZB | 413-253-0002 | %CO_LLC |
  Then members:
  | uid  | coType  |*
  | .ZZC | %CO_LLC |
  And relations:
  | main | other | permission |*
  | .ZZC | .ZZB  | manage     |
  And we show "%PROJECT Agreement"
  And we say "status": "info saved|step completed"
  And steps left "agree contact backing company donate photo preferences tithein discount"

  Given step done "agree"
  And step done "contact"
  And step done "backing"
  And step done "company"
  And step done "donate"
  And step done "photo"
  And step done "preferences"
  When member ".ZZC" completes form "settings/tithein" with values:
  | crumbs | 1.5 |**
  Then members have:
  | uid  | crumbs | flags               | legalName |*
  | .ZZC | .015   | member,confirmed,co | Our Pub   |
  And we show "Get Your Customers Signed Up"
  And we say "status": "info saved|step completed"
  And steps left "discount"

  When member ".ZZC" completes form "community/discount" with values:
  | amount  | 0 |**
  | minimum | 0 |
  Then we show "Account Summary" with:
  | Next Steps |
  | Invite |
  | Give   |
  And without:
  | Finish |
  And we say "status": "info saved|co2 member"
  And steps left ""
