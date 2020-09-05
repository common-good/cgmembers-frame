Feature: Post
AS a person
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
  | zip         | 01001        | 01001        |
  | phone       | +14132530001 | +4132530004  |
  | email       | a@b.c        | d@           |
  | method      | text         | email        |
  | confirmed   | 0            | 1            |
  | latitude    | 42.5         | 42.5         |
  | longitude   | -72.8        | -72.8        |
  And members:
  | uid         | .ZZE         | .ZZF         |**
  | fullName    | Eve Five     | Flo Six      |
  | address     | 5 E St.      | 6 F St.      |
  | city        | Eville       | Fville       |
  | state       | MA           | MA           |
  | zip         | 01005        | 01006        |
  | phone       | +14132530005 | +4132530006  |
  | email       | e@           | f@           |
  | latitude    | 42.5         | 42.5         |
  | longitude   | -72.8        | -72.8        |
  | flags       | ok           | ok           |
  And member ".ZZE" has "person" steps done: "contact"

Scenario: Someone visits the posts page
  When someone visits "community/posts"
  Then we show "Offers, Needs, & Tips" with:
  | Where  |    |    |
  | Radius | 10 | Go |
  And without:
  | Post a   |

Scenario: Someone submits a locus
  When someone confirms "community/posts" with:
  | locus          | radius | latitude | longitude |*
  | Greenfield, MA | 10     | 0        | 0         |
  Then we show "Offers, Needs, & Tips" with:
  | List View | Post |
#  And with:
#  | Needs | Offers |
  And with:
  | Item | Details | |
  And with:
  | There are not yet any needs within  |
  | There are not yet any offers within |
  | There are not yet any tips within |
  And cookie "locus" is "Greenfield, MA"
  And cookie "radius" is "10"
  And cookie "latitude" is "42.3791167"
  And cookie "longitude" is "-73.2819463"
  And cookie "zip" is "01301"

Scenario: Someone posts an offer
  When someone visits "community/posts/op=post"
  Then we show "Post Something" with:
  | Type |
  | Category |
  | What |
  | Details |
  | Emergency |
  | Radius |
  | End |
  | Your Email |
  
  When someone confirms "community/posts/op=post" with:
  | type  | cat  | item | details | emergency | radius | end     | email |*
  | offer | food | fish | big one | 1         | 3      | %mdY+3d | x@    |
  Then we show "Your Information" with:
  | Name |
  | Display Name |
  | Street Address |
  | City |
  | State |
  | Postal Code |
  | Phone |
  | Preferred Contact |
  And cookie "radius" is "3"
  And cookie "email" is "x@example.com"
  
Scenario: Someone enters personal data after posting an offer
  When someone confirms "community/posts/op=who&cat=1&item=fish&details=big one&emergency=1&radius=3&end=%now+3d&email=b@c.d&type=offer&exchange=0" with:
  | fullName    | Bea Two      |**
  | displayName | Bea          |
  | address     | 2 B St.      |
  | city        | Bville       |
  | state       | MA           |
  | zip         | 01002        |
  | phone       | 413-253-0002 |
  | method      | text         |
  | days        | 2            |
  | washes      | 3            |
  | health      | 2            |
  Then these "posts":
  | postid | type  | item | details | cat  | exchange | emergency | radius | pid | created | end     |* 
  | 1      | offer | fish | big one | food | 0        | 1         | 3      | 5   | %today  | %now+3d |
  And these "people":
  | pid         | 5            |**
  | fullName    | Bea Two      |
  | displayName | Bea          |
  | address     | 2 B St.      |
  | city        | Bville       |
  | state       | MA           |
  | zip         | 01002        |
  | phone       | +14132530002 |
  | email       | b@c.d        |
  | method      | text         |
  | confirmed   | 0            |
  | health      | 2 3 ok       |  
  And we email "confirm-post" to member "b@c.d" with subs:
  | fullName | item | date | thing | code | noFrame |*
  | Bea Two  | fish | %mdY | post  |    ? |       1 |
  And we say "status": "confirm by email" with subs:
  | thing | post |**

Scenario: A member enters data after posting an offer
  When someone visits "community/posts/op=who&cat=1&item=fish&details=big one&emergency=1&radius=3&end=%now+3d&email=e@example.com&type=offer&exchange=0"
  Then we show "Your Information" with:
  | Display Name |
  | Preferred Contact |
# also covid questions
  And without:
#  | Name |
  | Street Address |
  | City |
  | State |
  | Postal Code |
  | Phone |

  When someone confirms "community/posts/op=who&cat=1&item=fish&details=big one&emergency=1&radius=3&end=%now+3d&email=e@example.com&type=offer&exchange=0" with:
  | displayName | Eve          |**
  | method      | text         |
  | days        | 2            |
  | washes      | 3            |
  | health      | 2            |
  Then these "posts":
  | postid | type  | item | details | cat  | exchange | emergency | radius | pid | created | end     |* 
  | 1      | offer | fish | big one | food | 0        | 1         | 3      | 5   | %today  | %now+3d |
  And these "people":
  | pid         | 5            |**
  | fullName    | Eve Five     |
  | displayName | Eve          |
  | uid         | .ZZE         |
  | address     | 5 E St.      |
  | city        | Eville       |
  | state       | MA           |
  | zip         | 01005        |
  | phone       | +14132530005 |
  | email       | e@           |
  | method      | text         |
  | confirmed   | 0            |
  | health      | 2 3 ok       |  
  And we email "confirm-post" to member "e@" with subs:
  | fullName | item | date | thing | code | noFrame |*
  | Eve Five | fish | %mdY | post  |    ? |       1 |
  And we say "status": "confirm by email" with subs:
  | thing | post |**

Scenario: Someone confirms an offer once, twice
  Given these "posts":
  | postid | type  | item | details | cat  | exchange | emergency | radius | pid | created   | end     |* 
  | 1      | offer | fish | big one | food | 0        | 1         | 3      | 1   | %today-1d | %now+3d |
  When someone visits "community/posts/op=confirm&thing=post&code=%code" where code is:
  | postid | created   |*
  | 1      | %today-1d |
  Then these "people":
  | pid | confirmed |*
  | 1   | 1         |
  And these "posts":
  | postid | confirmed |*
  | 1      | 1         |
  And we say "status": "post success"
  
  When someone visits "community/posts/op=confirm&thing=post&code=%code" where code is:
  | postid | created   |*
  | 1      | %today-1d |
  Then we redirect to "community/posts/op=show&postid=1"
  And we show "Edit Post" with:
  | Category:  | food |
  | Who:       | Abe |
  | Posted:    | %mdY-1d |
  | What:      | fish |
  | Details:   | big one |
  |            | Max 500 characters |
  | Emergency: | |
  | Radius:    | 10 miles |
  | End Date:  | %mdY+3d |
  | Update     | |

  When someone confirms "community/posts/op=show&postid=1" with:
  | type | cat    | item   | details | emergency | radius | end     |*
  | tip  | health | Boston | ASAP    | 1         | 5      | %mdY+5d |
  Then these "posts":
  | postid | type | cat    | item | details | exchange | emergency | radius | pid | created    | end          |* 
  | 1      | tip  | health | Boston | ASAP  | 0        | 1         | 5      | 1   | %today-1d | %daystart+5d |
  And we show "Offers, Needs, & Tips" with:
  | List View | Post |
#  | Needs | Offers | Tips |
  And we say "status": "info saved"

Scenario: Someone views the details of an offer
  Given these "posts":
  | postid | type  | item | details | cat  | exchange | emergency | radius | pid | confirmed | created | end     |* 
  | 1      | offer | fish | big one | food | 0        | 1         | 3      | 1   | 1         | %now    | %now+3d |
  When someone visits "community/posts/op=show&postid=1"
  Then we show "Details" with:
  | Category        | food |
  | Who             | Abe |
  | Offer           | (In emergency) fish |
  | Details         | big one |
  | Message to Send | Max 200 characters |
  | Your Email      | |

Scenario: Someone views the details of an urgent need
  Given these "posts":
  | postid | type | item | details | cat  | exchange | emergency | radius | pid | created | end     |* 
  | 1      | need | fish | big one | food | 0        | 1         | 3      | 1   | %now    | %now+3d |
  When someone visits "community/posts/op=show&postid=1"
  Then we show "Details" with:
  | Category        | food |
  | Who             | Abe |
  | Urgent Need     | fish |
  | Details         | big one |
  | Message to Send | Max 200 characters |
  | Your Email      | |

Scenario: Someone views the details of a tip
  Given these "posts":
  | postid | type | item | details | cat  | exchange | emergency | radius | pid | created | end     |* 
  | 1      | tip  | fish | big one | food | 0        | 1         | 3      | 1   | %now    | %now+3d |
  When someone visits "community/posts/op=show&postid=1"
  Then we show "Details" with:
  | Category | food                |
  | Who      | Abe                 |
  |          | Aville, MA          |
  | Tip      | (In emergency) fish |
  | Details  | big one             |
And without:
  | Message |
And without:
  | Your Email |
  
Scenario: Someone replies to an offer
  Given these "posts":
  | postid | type  | item | details | cat  | exchange | emergency | radius | pid | created | end     | confirmed |* 
  | 1      | offer | fish | big one | food | 0        | 1         | 26     | 1   | %now    | %now+3d | 1         |
  And cookie "locus" is "Greenfield, MA"
  And cookie "radius" is "100"
  And cookie "latitude" is "42.3791167"
  And cookie "longitude" is "-73.2819463"
  And cookie "zip" is "01301"

  When someone visits "community/posts"
  Then we show "Offers, Needs, & Tips" with:
  | Where    | Greenfield, MA |    |
  | Radius   | 100   | Go |

  When someone confirms "community/posts" with:
  | locus          | radius | latitude | longitude |*
  | Greenfield, MA | 100    | 0        | 0         |
  Then we show "Offers, Needs, & Tips" with:
  | List View | Post |
#  | Needs | Offers |
  And with:
  |          | Item    | Details | |
  | food     | !! fish | big one | |

  When someone confirms "community/posts/op=show&postid=1" with:
  | email | message      |*
  | b@c.d | Hello there! |
  Then we show "Your Information" with:
  | Name |
  | Display Name |
  | Street Address |
  | City |
  | State |
  | Postal Code |
  | Phone |
  | Preferred Contact |
  And cookie "email" is "b@c.d "

Scenario: Someone enters personal data after replying to an offer
  Given these "posts":
  | postid | type  | item | details | cat  | exchange | emergency | radius | pid | created | end     | confirmed |* 
  | 1      | offer | fish | big one | food | 0        | 1         | 3      | 1   | %now    | %now+3d | 1         |
  When someone confirms "community/posts/op=who&email=b@c.d&message=Hello there!&postid=1" with:
  | displayName | Bea |**
  | fullName    | Bea Two |
  | address     | 2 B St. |
  | city        | Greenfield |
  | state       | MA |
  | zip         | 01301 |
  | phone       | 413-253-0002 |
  | method      | email |
  | days        | 2 |
  | washes      | 3 |
  | health      | 2 |
  Then these "messages":
  | id | postid | sender | message      | created | confirmed |*
  | 1  | 1      | 5      | Hello there! | %now    | 0         |
  And these "people":
  | pid | displayName | fullName | address | city     | state | zip   | phone     | email | method | confirmed | health |*
  | 5   | Bea         | Bea Two  | 2 B St. | Greenfield | MA  | 01301 | +14132530002 | b@c.d | email  | 0      | 2 3 ok |
  And we email "confirm-message" to member "b@c.d" with subs:
  | fullName | item | date | thing   | code | noFrame | what     |*
  | Bea Two  | fish | %mdY | message |    ? |       1 | an offer |
  And we say "status": "confirm by email" with subs:
  | thing | message |**

  When someone visits "community/posts/op=confirm&thing=message&code=%code" where code is:
  | id | created | location                      |*
  | 1  | %now    | Greenfield, MA 01301 (0.2 mi) |
  Then we email "post-message" to member "a@b.c" with subs:
  | fullName | item | date | thing | message      | fromLocation                  | noFrame |*
  | Abe One  | fish | %mdY | post  | Hello there! | Greenfield, MA 01301 (0.2 mi) |      1 |
  And we say "status": "message sent"

Scenario: Someone confirmed sends a message and posts again
  Given these "people":
  | pid | displayName | fullName | address | city     | state | zip   | phone        | email | method | confirmed |*
  | 2   | Bea         | Bea Two  | 2 B St. | Greenfield | MA  | 01301 | +14132530002 | b@c.d | email  | 1         |
  And these "posts":
  | postid | type  | item | details | cat  | exchange | emergency | radius | pid | created | end     | confirmed |* 
  | 1      | offer | fish | big one | food | 0        | 1         | 3      | 1   | %now    | %now+3d | 1         |
  And cookie "vipid" is 2
  And cookie "email" is "b@c.d"

  When someone confirms "community/posts/op=show&postid=1" with:
  | email | message      |*
  | b@c.d | Hello there! |
  
  Then these "messages":
  | id | postid | sender | message      | created | confirmed |*
  | 1  | 1      | 2      | Hello there! | %now    | 1         |
  
  When someone confirms "community/posts/op=post" with:
  | type | cat   | item | details | emergency | exchange | radius | end     | email |*
  | need | stuff | bag  | paper   | 0         | 1        | .25    |         | b@c.d |  

  Then these "posts":
  | postid | type | item | details | cat   | emergency | exchange | radius | pid | created | end |* 
  | 2      | need | bag  | paper   | stuff | 0         | 1        | .25    | 2   | %today  |     |
