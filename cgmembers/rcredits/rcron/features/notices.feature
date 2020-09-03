Feature: Notices
AS a member
I WANT to hear about what's going on in my account
SO I can take appropriate action

Setup:
  Given members:
  | uid  | fullName   | flags     | email |*
  | .ZZA | Abe One    | ok        | a@    |
  | .ZZB | Bea Two    | member,ok,weekly | b@    |
  | .ZZC | Corner Pub | co,ok     | c@    |
  And these "people":
  | pid         | 5            | 4            |**
  | fullName    | Eve Five     | Dee Four     |
  | displayName | Eve          | Dee          |
  | address     | 5 E St.      | 4 D St.      |
  | city        | Eville       | Dville       |
  | state       | MA           | MA           |
  | zip         | 01001        | 01001        |
  | phone       | +14132530005 | +4132530004  |
  | email       | e@           | d@           |
  | method      | text         | email        |
  | confirmed   | 0            | 1            |
  | latitude    | 42.5         | 42.5         |
  | longitude   | -72.8        | -72.8        |
  | notices     | %NOTICE_DFTS | offer:d,need:w,tip:m |
  And community email for member ".ZZA" is "%whatever@rCredits.org"

Scenario: a member gets some notices
  Given notices:
  | uid  | created | sent | message    |*
  | .ZZA | %today  |    0 | You rock.  |
  | .ZZA | %today  |    0 | You stone. |
  When cron runs "notices"
  Then we email "notices" to member "a@" with subs:
  | fullName | shortName | unit | range   | yestertime | region | messages                  | balance  | savings | ourEmail      |*
  | Abe One  | abeone    | day  | %dmy-1d | %dmy-1d    | new    | *You rock.<br>*You stone. | $0       | $0      | %whatever@rCredits.org |
  And notices:
  | uid  | created | sent   | message    |*
  | .ZZA | %today  | %today | You rock.  |
  | .ZZA | %today  | %today | You stone. |

Scenario: a member gets some weekly notices
  Given notices:
  | uid  | created | sent | message    |*
  | .ZZB | %today  |    0 | You rock.  |
  | .ZZB | %today  |    0 | You stone. |
  And it's time for "weekly"
  When cron runs "notices"
  Then we email "notices" to member "b@" with subs:
  | fullName | shortName | unit | range               | yestertime | region | messages                            | balance  | savings | ourEmail      |*
  | Bea Two  | beatwo    | week | the week of %dmy-1w | %dmy-1w    | new    | * %md<x>You rock.<br>* %md<x>You stone. | $0       | $0      | %whatever@rCredits.org |
  And notices:
  | uid  | created | sent   | message    |*
  | .ZZB | %today  | %today | You rock.  |
  | .ZZB | %today  | %today | You stone. |

Scenario: a member gets post notices
  Given these "posts":
  | postid | type  | item | details | cat  | exchange | emergency | radius | pid | created | end     | confirmed |* 
  | 1      | offer | fish | big one | food | 0        | 1         | 3      | 5   | %today  | %now+3d | 1         |
  When cron runs "notices"
  Then we email "post-notice" to member "d@" with subs:
  | fullName | posts | radius | code | noFrame |*
  | Dee Four | ?     | 20     | ?    | 1       |