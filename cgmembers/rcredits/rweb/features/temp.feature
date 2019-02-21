Feature: A user signs up for rCredits
AS a newbie
I WANT to open an rCredits account
SO I can be part of the Common Good Economy
# Note that "member" in the scenarios below means new member (newbie).

Setup:
  Given members:
  | uid  | fullName | acctType    | flags      | created  | email |*
  | .ZZZ | Zeta Zot | personal    | ok         | 99654321 | zot@  |
  | NENAAA | New Hampshire | community |       | 99654321 | nen@  |
  And member is logged out


Scenario: A newbie registers from elsewhere
  Given invitation to email "a@" from member ".ZZZ" is "c0D3"
  And next random code is "WHATEVER"
  When member "?" confirms form "signup/code=c0D3" with values:
  | fullName | email | phone       | zip | federalId   | dob      | acctType    | address | city | state | postalAddr                   | tenure | owns | helper |*
  | Abe One  | a@ | (333) 253-0000 | 03768-2345 | 111-22-3333 | 1/2/1990 | %CO_PERSONAL | 1 A ST. | Lyme | NH    | 1 A ST., Lyme, NH 03768-2345 |     18 |    1 | .ZZZ   |
 Then members:
  | uid    | fullName | email | phone     | zip | state | city | flags     | helper |*
  | NENAAB | Abe One  | a@ | +13332530000 | 03768-2345 | NH    | Lyme | confirmed | .ZZZ   |
  And we show "Verify Your Email Address"
  And we say "status": "info saved|step completed"
  And we email "tell-staff" to member "nen@" with subs:
  | fullName |*
  | Abe One  |
  And we email "verify" to member "a@" with subs:
  | fullName | name   | quid    | site        | code  |*
  | Abe One  | abeone | NEN.AAB | %BASE_URL | WHATEVER |
  # And we show "Empty"
