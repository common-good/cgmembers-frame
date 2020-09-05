Feature: Notices
AS a member
I WANT to hear about what's going on in my account
SO I can take appropriate action

Setup:
  Given members:
  | uid  | fullName   | flags     | email | notices      |*
  | .ZZA | Abe One    | ok        | a@    | offer:d,need:d,tip:w,in:w,out:d,misc:m |
  | .ZZB | Bea Two    | member,ok | b@    | offer:d,need:d,tip:w,in:w,out:d,misc:m |
  | .ZZC | Corner Pub | co,ok     | c@    | %NOTICE_DFTS |
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
  | notices     | offer:d,need:w,tip:m | offer:d,need:w,tip:m |
  And community email for member ".ZZA" is "%whatever@cg.org"

Scenario: a member gets some notices
  Given notices:
  | uid  | created | sent | message    | type |*
  | .ZZA | %today  |    0 | You rock.  | out  |
  | .ZZA | %today  |    0 | You stone. | out  |
  When cron runs "notices"
  Then we email "notices" to member "a@" with subs:
  | fullName   | Abe One |**
  | range      | Transfers OUT (%mdY-1d) |
  | region     | new     |
  | messages   | *You rock.<br>*You stone. |
  | balance    | $0 |
  | ourEmail   | %whatever@cg.org |
  | code       | ? |
  And notices:
  | uid  | created | sent   | message    |*
  | .ZZA | %today  | %today | You rock.  |
  | .ZZA | %today  | %today | You stone. |

Scenario: a member gets some weekly notices
  Given notices:
  | uid  | created | sent | message    | type |*
  | .ZZB | %today  |    0 | You rock.  | in   |
  | .ZZB | %today  |    0 | You stone. | out  |
  And it's time for "weekly"
  When cron runs "notices"
  Then we email "notices" to member "b@" with subs:
  | fullName   | Bea Two |**
  | range      | Transfers IN (week of %mdY-1w) and Transfers OUT (%mdY-1d) |
  | region     | new     |
  | messages   | * %md<x>You rock.<br>* %md<x>You stone. |
  | balance    | $0 |
  | ourEmail   | %whatever@cg.org |
  | code       | ? |
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
  And we do not email "post-notice" to member "e@"

Scenario: a member gets only today's post notices
  Given these "posts":
  | postid | type  | item | details | cat  | exchange | emergency | radius | pid | created | end     | confirmed |* 
  | 1      | tip   | fish | big one | food | 0        | 1         | 3      | 5   | %today  | %now+3d | 1         |
  When cron runs "notices"
  Then we do not email "post-notice" to member "d@"
  And we do not email "post-notice" to member "d@"
