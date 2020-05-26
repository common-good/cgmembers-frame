Feature: Invite
AS a member
I WANT to invite other people to join
SO we can control our local economy democratically
 
Setup:
  Given members:
  | uid  | fullName        | floor | flags           | city | zip   |*
  | .ZZA | Abe One         |  -250 | ok,confirmed    | Aton | 01000 |
  | .ZZB | Bea Two         |  -250 |                 | Bton | 01000 |
  | .ZZC | Our Pub         |  -250 | ok,confirmed,co | Cton | 01000 |

Scenario: A member invites
  When member ".ZZB" visits page "community/invite"
  Then we show "Invite People"

  When member ".ZZB" completes form "community/invite" with values:
  | sign | quote | org  | position | website | usePhoto |*
  | 1    | cuz   | MeCo | Boss     | me.co   | 1        |
  Then these "u_shouters":
  | uid  | quote | org  | title | website | usePhoto |*
  | .ZZB | cuz   | MeCo | Boss  | me.co   | 1        |
  
  When member ".ZZA" visits page "community/invite/all"
  Then we show "Please sign up now" with:
  | Bea Two | Boss | MeCo | cuz |

  When member ".ZZB" visits page "community/invite"
  Then we show "Invite Someone"
  When member ".ZZB" completes form "community/invite" with values:
  | email | trusted | zip   | subject | message |*
  | a@    |       1 | 01002 | test    | hi!     |
  Then these "invites":
  | inviter | invited | zip   | subject | message |*
  | .ZZB    | %today  | 01002 | test    | hi!     |
