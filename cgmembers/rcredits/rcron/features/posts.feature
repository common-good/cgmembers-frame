Feature: Post
AS a participant
I WANT to exchange free help with my neighbors
SO we can all thrive together.

Setup:
  Given these "people":
  | pid         | 1            | 4            |**
  | fullName    | Abe One      | Dee Four     |
  | displayName | Abe          | Dee          |
  | address     | 1 A St.      | 4 D St.      |
  | city        | Aville       | Dville       |
  | state       | MA           | MA           |
  | zip         | 01001        | 04004        |
  | phone       | +14132530001 | +4132530004  |
  | email       | a@b.c        | d@           |
  | method      | text         | email        |
  | confirmed   | 0            | 1            |
  | latitude    | 42.5         | 42.5         |
  | longitude   | -72.8        | -72.8        |
  Given these "posts":
  | postid | type  | item | details | cat  | service | exchange | emergency | radius | pid | created   | end | confirmed |* 
  | 1      | offer | fish | big one | food | 0       | 0        | 0         | .01    | 1   | %now-360d |     | 1 |
  | 2      | offer | duck | big two | food | 0       | 0        | 0         | .01    | 4   | %now-1d   |     | 1 |

Scenario: a participant gets a notice about an expiring post
  When cron runs "notices"
  Then we email "post-expires" to member "a@b.c" with subs:
  | fullName   | Abe One |**
  | code       | ?       |
  | noFrame    | 1       |
  | item       | fish    |
  | grace      | 5       |
  | dt         | %mdY+5d |
