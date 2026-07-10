Wokwi links for submission:

1) phase 2 final submission : https://wokwi.com/projects/468811747411795969
2) phase 1 midway submission : https://wokwi.com/projects/468729759776562177

EXPLANTION OF CODE:

There is mainly two states in my code which are controlled by the variable "lock" which is the Timeout state , the timeout state will trigger when 3 wrong consecutive codes will be entered and it will last for 15 seconds.
The HASHTAG key is the BACKSPACE KEY in the keypad (#) , the asterisk is a dummy key in the keypad(*).
The keys pressed will be inputted as char , so explicity converting the keys to numbers by : int(key) -48 , minus 48 is because numbers have an ASCII value fro 48 , zero has 48 value so 1 will be converted to 49 and so on..
Then , after conversion , storing the code in variable "codeValue" to compare it with the user codes later.

Backspace key working : 

When hastag is pressed , the "cursor" variable is decremented by one which stores the no of digits entered and that many asterisk is printed in the LED. Also "codeValue" variable is int so dividing by 10 will just remove the unit's digit number in it.
Then calling the checkCode function , it compares the codes and if correct code is entered it fetches the current time and calls the "sendpost" function.

"Sendpost" function working :

the function first checks the wifi connection, if wifi is connected it will send all the stored entries by http post and then the current entrie also . if wifi is disconnected at that time, it will strore the entriy in an OFLINE STRUCT ARRAY.
Then the code will return to the void loop and continue from there by clearing the lcd screen and setting the variables codevalue etc.. to zero for next iteration.

OFFLINE QUEUE working :

I have created a Struct Array of size 10 structs which can store upto 10 code entries. struct has the name of user , code and time of entry. When the wifi is disconnected and "sendpost" function is called it will store the entry in this array and then connect to wifi in background by wifi.begin() command. As soon as the wifi connects , all these stored logs will be sent by http post and then new entries can be stored from index zero again.

Avoiding heap fragmentation : 

Whenever before creating a new jsonstring (serialisation) , I have put jsonString="" , this makes it an empty string otherwise it will just append the new codes and multiple codes will be sent again by http post. Also i am creating Json document "myjson" at the start i.e. globally and then before each serialisation I have put myjson.clear() . This will not let the computer create memory space for json document again and again thus avoiding heap fragemntation.
