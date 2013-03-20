# this message contains information to define the comunication between the Decision making (DM) and UI_Pri when a user seleccs an object

string solution   # currently supported comands from the user are:yes,no. There is timeout which results in no_select  

uint32 responseID  # the uniqie ID used by DM to distinguish which responce corresponds to which command. Note: the responceID MUST be the same as the requestID of the corresponding command
