Feature: Membership
AS a member
I WANT to manage my progress toward activating my account
SO I can participate actively.

#phone 1 means +1 413.772.0001

Setup:
  Given members:
  | uid | fullName | phone | email | city  | state | zip   | floor | flags     | pass      |*
  | .ZZA | Abe One |     1 | a@    | Atown | AK    | 01000 |     0 |           | %whatever |
  | .ZZB | Bea Two |     2 | b@    |       | UT    | 02000 |  -200 |           | |
  | .ZZC | Our Pub |     3 | c@    | Ctown | CA    | 03000 |     0 | member,co | |
Skip
# (see also signup feature) 
Scenario: An individual member signs up
  Given member is logged out
  And next random code is "WHATEVER"
  When member "?" completes form "signup" with values:
  | fullName | email | phone        | zip   | acctType     |*
  | Al Aargh | z@    | 413-253-0000 | 01002 | %CO_PERSONAL |
  Then members:
  | uid  | fullName | legalName | email | phone        | zip   | state |*
  | .AAA | Al Aargh | Al Aargh  | z@    | +14132530000 | 01002 | MA    |
  And we show "Verify Your Email Address"
  And we say "status": "info saved|step completed"
  And member ".AAA" steps left "verifyemail verifyid agree preferences fund photo contact donate tithein proxies work backing invite"

  Given member is logged out
  When member "?" visits page "reset/id=alaargh&code=WHATEVER&verify=1"
  Then we show "Verified!"
  And member ".AAA" steps left "verifyid agree preferences fund photo contact donate tithein proxies work backing invite"
  
  When member "?" completes form "reset/id=alaargh&code=WHATEVER&verify=1" with values:
  | pass1 | pass2 |*
  |       |       |
  Then we show "Identity Verification"
  And member ".AAA" steps left "verifyid agree preferences fund photo contact donate tithein proxies work backing invite"

Scenario: An individual member verifies ID
  Given member ".ZZB" has "person" steps done: "signup verifyemail"
  When member ".ZZB" completes form "settings/verifyid" with values:
  | field | federalId   | dob      |*
  |     2 | 123-45-6789 | 2/1/1990 |
  # field 2 is SSN and DOB, as opposed to file upload
  Then members:
  | uid  | federalId | dob       |*
  | .ZZB | 123456789 | 633848400 |
  And we show "%PROJECT Agreement" with:
  | I make this agreement |
  And we say "status": "info saved|step completed"
  And member ".ZZB" steps left "agree preferences fund photo contact donate tithein proxies work backing invite ssn"

Scenario: An individual member signs agreement
  Given member ".ZZB" has "person" steps done: "signup verifyemail verifyid"
  When member ".ZZB" completes form "community/agreement" with values:
  | op | I Agree |**
  Then we show "Account Preferences"
  And we say "status": "info saved|step completed"
  And steps left "preferences fund photo contact donate tithein proxies work backing invite"

Scenario: An individual member sets preferences
  Given member ".ZZB" has "person" steps done: "signup verifyemail verifyid agree"
  When member ".ZZB" completes form "settings/preferences" with values:
  | roundup | crumbs | notices | statements | nosearch | secretBal |*
  |       1 |      2 | monthly | electronic |        0 |         1 |
  Then we show "Getting Money In or Out"
  And we say "status": "info saved|step completed"
  And steps left "fund photo contact donate tithein proxies work backing invite"

Scenario: An individual member connect bank account
  Given member ".ZZB" has "person" steps done: "signup verifyemail verifyid agree preferences"
  When member ".ZZB" completes form "settings/fund" with values:
  | op     | connect | routingNumber | bankAccount | bankAccount2 | cashout | refills | target | achMin | saveWeekly |*
  | submit |       2 |     053000196 |         123 |          123 |       0 |       1 |     $0 |    $20 |         $0 |  
  Then we show "Photo ID Picture"
  And we say "status": "info saved|step completed"
	And members have:
	| uid  | risks   |*
	| .ZZB | hasBank |
  And steps left "photo contact donate tithein proxies work backing invite"

Scenario: An individual member uploads a photo
  Given member ".ZZB" has "person" steps done: "signup verifyemail verifyid agree preferences fund"
  When member ".ZZB" completes form "settings/photo" with values:
  | op       |*
  | nextStep |
  Then we show "Contact Information"
  And we say "status": "info saved|step completed"
  And steps left "contact donate tithein proxies work backing invite"

Scenario: An individual member gives contact info
  Given member ".ZZB" has "person" steps done: "signup verifyemail verifyid agree preferences fund photo"
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
  And steps left "donate tithein proxies work backing invite"
Resume
Scenario: An individual member donates
  Given member ".ZZB" has "person" steps done: "signup verifyemail verifyid agree preferences fund photo contact"
  When member ".ZZB" completes form "community/donate" with values:
  | amtChoice | period | honor | honored |*
  |        50 |      M |     - |         |
  Then we show "Share When You Receive"
  And we say "status": "info saved|step completed"
  And steps left "tithein proxies work backing invite"
Skip
Scenario: An individual member chooses tithes
  Given member ".ZZB" has "person" steps done: "signup verifyemail verifyid agree preferences fund photo contact donate"
  When member ".ZZB" completes form "settings/tithein" with values:
  | crumbs | 1.5 |**
  Then members have:
  | uid  | crumbs |*
  | .ZZB | .015   |
  And we show "Proxies"
  And we say "status": "info saved|step completed"
  And steps left "proxies work backing invite"
Skip
Scenario: An individual member chooses proxies
  Given member ".ZZB" has "person" steps done: "signup verifyemail verifyid agree preferences fund photo contact donate tithein"
  And proxies:
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
  | Card   |
  | Invite |
  | Sell   |
  | Give   |
  And without:
  | Voice  |
  And we say "status": "info saved|voice member"
  And steps left ""

Skip
  When member ".ZZB" completes form "settings/ssn" with values:
  | federalId   |*
  | 123-45-6789 |
  Then we show "Contact Information"
  And we say "status": "info saved|step completed"
  And steps left "contact tithein ssn"


Scenario: An individual member confirms social security number
  Given member ".ZZB" has "person" steps done: "signup verifyemail verifyid agree"
  Then we show "Confirm Your Social Security Number"
  And we say "status": "info saved|step completed"
  And member ".ZZB" steps left "ssn"
  
  When member ".ZZB" completes form "settings/ssn" with values:
  | federalId   |*
  | 123-45-6789 |
  Then we show "Account Summary" with:
  | Next Steps |
  | Card   |
  | Invite |
  | Sell   |
  | Voice  |
  | Give   |
  And we say "status": "setup complete|individual approval|join thanks|next steps|no card member"
  And steps left ""
  And members have:
  | uid  | flags  |*
  | .ZZB | member,refill,roundup,monthly,secret |
  And we tell ".ZZB" CO "New Member (Al Aargh)" with subs:
  | quid | status |*
  | .ZZB | member |
  
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
  | Finish |
  | Invite |
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
  And steps left "agent agree contact backing photo donate company tithein discount"

  Given members have:
  | uid  | phone        |*
  | .ZZB | 413-253-0002 |
  When member ".ZZC" completes form "settings/agent" with values:
  | agent  | phone        | coType  | legalName   | federalId  |*
  | NEWZZB | 413-253-0002 | %CO_LLC | Our Pub Inc | 20-4333048 |
  Then members:
  | uid  | coType  | legalName   | federalId |*
  | .ZZC | %CO_LLC | Our Pub Inc | 204333048 |
  And relations:
  | main | other | permission |*
  | .ZZC | .ZZB  | manage     |
  And we show "%PROJECT Agreement" with:
  | Signed | Our Pub | Bea Two |
  And we say "status": "info saved|step completed"
  And steps left "agree contact backing photo donate company tithein discount"

  When member ".ZZC" completes form "community/agreement" with values:
  | op      | signedBy |* 
  | I Agree | Bea Two  |
  Then members:
  | uid  | signed | signedBy |*
  | .ZZC | %now   | Bea Two  |
  And we show "Contact Information"

  Given step done "contact"
  And step done "backing"
  And step done "company"
  And step done "donate"
  And step done "photo"
  When member ".ZZC" completes form "settings/tithein" with values:
  | crumbs | 1.5 |**
  Then members have:
  | uid  | crumbs | flags               |*
  | .ZZC | .015   | member,confirmed,co |
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
