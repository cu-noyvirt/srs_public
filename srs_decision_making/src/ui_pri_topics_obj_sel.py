#!/usr/bin/python
# ROS imports


import roslib; roslib.load_manifest('srs_decision_making')
import rospy
import srs_decision_making.srv as xsrv
import srs_msgs.msg as srs_msg

class UI_PRI_TOPICS_OBJ_SEL:
    
    #### Initialization and declaration of the variables ####
    def __init__(self):
        rospy.loginfo("Public topics for UI_PRI_OBJ_SEL ...")
        self.pubUIobjsel = rospy.Publisher('DM_UI/interface_cmd_obj',srs_msg.DM_UIobjcom)
        self.user_respobjsol =""
        self.user_respobjpar =""
        self.user_comobj_id = 0
        rospy.Subscriber ("DM_UI/interface_cmd_obj_response",srs_msg.UI_DMobjresp, self.callbackUIobj)

    ### Declaration of callback function for the object selection commands from the user interface
    def callbackUIobj (self,data):
        rospy.loginfo ("I heard %s %s %i from the UI_PRI",data.solution,data.parameter, data.responseID)
        if (data.responseID == self.user_comobj_id):
            self.user_respobjsol = data.solution
            self.user_respobjpar = data.parameter
            rospy.loginfo ("Match between responseId and requestID. Now user_respsol is:%s and user_resppar is:%s",self.user_respobjsol,self.user_respobjpar)
        else:
            print ("unfortunately the id:%s does not correspond to requestId:%s",self.user_comobj_id)

### the main part    
    def selectObjHandle(self,req):

        rospy.loginfo("The user will be invited to select a new object:")

        satisfaction_flag = False
        
        self.time_out_max = 30
        try:
            self.time_out_max = rospy.get_param("srs/common/max_time_out")
        except Exception, e:
            rospy.loginfo("Parameter Server not ready, use default value for max time_out")
        
        while (not satisfaction_flag):
            rospy.loginfo("Message sent to user to ask if he wants to select an object:")
            self.user_respobjsol = ""
            message1 = srs_msg.DM_UIobjcom ("ask","Do you want to select an object? ",5)
            self.user_comobj_id = 5
            self.pubUIobjsel.publish(message1)
            
            timeout=0
            while (self.user_respobjsol =="" and timeout < self.time_out_max):
                print "waiting for response",timeout," seconds from the user"
                timeout = timeout + 1
                print "user_respobjsol is :",self.user_respsol
                rospy.sleep(1)

                      
            print "the final user_respobjsol is.:",self.user_respobjsol


            if (self.user_respobjsol.strip()=="yes"):
                rospy.loginfo("The response from the user is: <<No>>.")

                self.user_respobjsol =""
                self.user_respobjpar = ""
                self.solfromUser.giveup = 0
                self.solfromUser.solution = "Yes"
                satisfaction_flag = True
                

            elseif (self.user_respobjsol.strip()=="no"):
                rospy.loginfo("The response from the user is: <<No>>.")
                satisfaction_flag = True
                self.solfromUser.giveup = 0
                self.solfromUser.solution = "No"

                


             else: 
                rospy.loginfo("There was no responce from the user or it didn't make sense")
                satisfaction_flag = True
                self.solfromUser.giveup = 1
                self.solfromUser.solution = "No"



        print(self.solfromUser)               
        return xsrv.answer_yes_no(self.solfromUser.solution,self.solfromUser.giveup)
    
if __name__ == '__main__':
    rospy.init_node('control_task_server')
    UI_PRI_OBJ_TOPIC_s = UI_PRI_TOPICS_OBJ_SEL()
    rospy.Service('object_selection', xsrv.object_selection, selectObjHandle)
    rospy.spin()