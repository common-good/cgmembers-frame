Feature: Message
AS a member
I WANT to send a message to another member or participating company
SO I can contact them without knowing their contact information and/or so I can send them secret information safely

Setup:
  Given members:
  | uid  | fullName | flags   |*
  | .ZZA | Abe One  | ok,confirmed      |
  | .ZZB | Bea Two  | ok,confirmed      |
  | .ZZC | Cor Pub  | ok,confirmed,co   |
  
Scenario: Synopsis of scenario
  When member ".ZZA" confirms "community/message" with:
  | to   | subject   | message   |*
  | .ZZB | Greetings | Hi there! |
  Then we email "" to member "b@" with subs:
  | subject   | body      | noFrame |*
  | Greetings | Hi there! | 1       |
  And we say "status": "Your message has been sent to Bea Two."
