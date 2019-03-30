#include <sys/stat.h>

#include <linux/input.h>
#include <linux/uinput.h>

#include <err.h>
#include <signal.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

#include <err.h>
#include <signal.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h> 
#include <string.h> 
#include <sys/types.h> 
#include <netinet/in.h> 
#include <sys/socket.h> 
#include <sys/wait.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>

#include <libcec/cecc.h>

#define MYPORT 3490    /* the port users will be connecting to */
#define FILE_NAME "/var/run/tv"

#define BACKLOG 10     /* how many pending connections queue will hold */

#define LEN(x) (sizeof(x) / sizeof(*(x)))

//#define DEBUG

#ifdef DEBUG
#define DPRINTF_X(t) printf(#t"=0x%x\n", t)
#define DPRINTF_D(t) printf(#t"=%d\n", t)
#define DPRINTF_U(t) printf(#t"=%u\n", t)
#define DPRINTF_S(t) printf(#t"=%s\n", t)
#define DPRINTF printf
#define DEBUG_DO(x) x
#else
#define DPRINTF_X(t)
#define DPRINTF_D(t)
#define DPRINTF_U(t)
#define DPRINTF_S(t)
#define DPRINTF(t)
#define DEBUG_DO(x)
#endif

struct map {
	unsigned cec;  /* HDMI-CEC code */
	unsigned code; /* uinput code */
};

#include "config.h"

int fd;
int die;

FILE *file_ptr;

/* global cec connection handle */
libcec_connection_t g_connection;
void onkeypress(void *cbparam, const cec_keypress *key);
void onlogmsg(void *cbparam, const cec_log_message *msg);
void oncommand(void *cbparam, const cec_command *cmd);
void onalert(void *cbparam, const libcec_alert type, const libcec_parameter param);
void cecHdmi(void);

ICECCallbacks callbacks = {
	.logMessage = onlogmsg,
	.commandReceived = oncommand,
	.alert = onalert,
    .keyPress = onkeypress,
};

libcec_configuration g_config = {
	.clientVersion = LIBCEC_VERSION_CURRENT,
	.serverVersion = LIBCEC_VERSION_CURRENT,
	.strDeviceName = "Engelity",
	.deviceTypes = {
		.types = {
			CEC_DEVICE_TYPE_RECORDING_DEVICE,
			CEC_DEVICE_TYPE_RESERVED,
			CEC_DEVICE_TYPE_RESERVED,
			CEC_DEVICE_TYPE_RESERVED,
			CEC_DEVICE_TYPE_RESERVED,
		},
	},
	.cecVersion = CEC_DEFAULT_SETTING_CEC_VERSION,
	.callbacks = &callbacks,
};


void setupuinput(void)
{
	struct uinput_user_dev uidev;
	unsigned int i;
	int ret;

	fd = open(upath, O_WRONLY | O_NONBLOCK);
	if (fd == -1)
		err(1, "open %s", upath);

	ret = ioctl(fd, UI_SET_EVBIT, EV_KEY);
	if (ret == -1)
		err(1, "key events");
	ret = ioctl(fd, UI_SET_EVBIT, EV_SYN);
	if (ret == -1)
		err(1, "sync events");

	/* Set available keys */
	for (i = 0; i < LEN(bindings); i++) {
		ret = ioctl(fd, UI_SET_KEYBIT, bindings[i].code);
		if (ret == -1)
			err(1, "set key 0x%x", bindings[i].code);
	}

	memset(&uidev, 0, sizeof(uidev));
	snprintf(uidev.name, UINPUT_MAX_NAME_SIZE, "Engelity");
	uidev.id.bustype = BUS_USB;
	uidev.id.vendor  = 0xdead;
	uidev.id.product = 0xbabe;
	uidev.id.version = 1;

	ret = write(fd, &uidev, sizeof(uidev));
	if (ret == -1)
		err(1, "write uidev");
	ret = ioctl(fd, UI_DEV_CREATE);
	if (ret == -1)
		err(1, "create dev");
}

void cleanuinput(void)
{
	int ret;

	ret = ioctl(fd, UI_DEV_DESTROY);
	if (ret == -1)
		err(1, "destroy dev");

	close(fd);
}

void sendevent(unsigned type, unsigned code, unsigned val)
{
	struct input_event ev;
	int ret;

//    DPRINTF("[sendevent]send event %d code 0x%x\n", code, code);
	memset(&ev, 0, sizeof(ev));
	ev.type = type;
	ev.code = code;
	ev.value = val;
	ret = write(fd, &ev, sizeof(ev));
	if (ret == -1)
		err(1, "send event type 0x%x code 0x%x", type, code);
}

void sendkeypress(unsigned ceccode)
{
	int i;

	for (i = 0; i < LEN(bindings); i++) {
		if (ceccode == bindings[i].cec) {
			sendevent(EV_KEY, bindings[i].code, 1);
			sendevent(EV_SYN, SYN_REPORT, 0);
			sendevent(EV_KEY, bindings[i].code, 0);
			sendevent(EV_SYN, SYN_REPORT, 0);
			return;
		}
	}
	warnx("unhandled code 0x%x", ceccode);
}


void onlogmsg(void *cbparam, const cec_log_message *msg)
{
    const char* strLevel="UNKNOWN";
    switch (msg->level)
    {
    case CEC_LOG_ERROR:
      strLevel = "ERROR:   ";
      break;
    case CEC_LOG_WARNING:
      strLevel = "WARNING: ";
      break;
    case CEC_LOG_NOTICE:
      strLevel = "NOTICE:  ";
      break;
    case CEC_LOG_TRAFFIC:
      strLevel = "TRAFFIC: ";
      break;
    case CEC_LOG_DEBUG:
      strLevel = "DEBUG:   ";
      break;
    default:
      break;
    }

    DEBUG_DO(printf("%s\t%s\n", strLevel, msg->message));
}
void onkeypress(void *cbparam, const cec_keypress *key)
{

	DPRINTF_X(key->keycode);
	DPRINTF_X(key->duration);
}

void oncommand(void *cbparam, const cec_command *cmd)
{
	printf("[oncommand] %02x %02x %02x ",cmd->initiator,cmd->destination,cmd->opcode);
    for(int i= 0; i< cmd->parameters.size; i++) {

        printf("%02x ",cmd->parameters.data[i]);
    }
        printf("\n");
#if 0
    if(cmd.opcode == 0x82) { // set source
        if((cmd.initiator == 0) &&  (cmd.parameters.data[0] ==0) && (cmd.parameters.data[1] ==0)) {
         printf("[oncommand] TV is going to TDT\n");
           // TV is back to DVB
            if( access( FILE_NAME, F_OK ) == -1 ) { // file does not exists
                cecHdmi();

//                libcec_set_active_source(g_connection, g_config.deviceTypes.types[0]);        
                printf("[oncommand] BACK to HDMI\n");
           }    
        }
    }
#endif
    if(cmd->opcode == 0x44) { //key pressed
        printf("sendkeypress %02x\n",cmd->parameters.data[0]);
		sendkeypress(cmd->parameters.data[0]);
    }
}

void onalert(void *cbparam, const libcec_alert type, const libcec_parameter param)
{
	return;
}

void cecHdmi(void)
{
#if 1
    libcec_set_active_source(g_connection, g_config.deviceTypes.types[0]);            
#else
    cec_command cec_cmd_hdmi;
    cec_cmd_hdmi.initiator = CECDEVICE_RECORDINGDEVICE1;
    cec_cmd_hdmi.destination = CECDEVICE_TV;
    cec_cmd_hdmi.opcode =CEC_OPCODE_ACTIVE_SOURCE;
    cec_cmd_hdmi.parameters.data[0]= 0x10;
    cec_cmd_hdmi.parameters.data[1]= 0x00;
    cec_cmd_hdmi.parameters.size= 02;
    cec_cmd_hdmi.opcode_set=1;
    cec_cmd_hdmi.transmit_timeout=10;
    libcec_transmit(g_connection,&cec_cmd_hdmi);
    libcec_set_active_source(g_connection, g_config.deviceTypes.types[0]);   
#endif
}

void setupcec(void)
{
	cec_adapter devs[10], *dev;
	int ret;
	int n;

	g_connection = libcec_initialise(&g_config);
	if (!g_connection)
		errx(1, "cec init");

	/* Initialize host stack */
	libcec_init_video_standalone(g_connection);

	/* Auto detect adapters */
	n = libcec_find_adapters(g_connection, devs, LEN(devs), NULL);
	if (n == -1)
		errx(1, "find adapters");
	if (n == 0)
		errx(1, "no adapters found");
	DPRINTF_D(n);

	/* Take the first */
	dev = &devs[0];
	DPRINTF_S(dev->path);
	DPRINTF_S(dev->comm);

	ret = libcec_open(g_connection, dev->comm, CEC_DEFAULT_CONNECT_TIMEOUT);
	if (!ret)
		errx(1, "open port %s", dev->comm);
}

void cleancec(void)
{
	libcec_close(g_connection);
	libcec_destroy(g_connection);
}

void death(int sig)
{
	die = 1;
}

void dotcp() 
{
    char buff[10];
    int sockfd, new_fd;  /* listen on sock_fd, new connection on new_fd */
    struct sockaddr_in my_addr;    /* my address information */
    struct sockaddr_in their_addr; /* connector's address information */
    unsigned int sin_size;

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        exit(1);
    }

    my_addr.sin_family = AF_INET;         /* host byte order */
    my_addr.sin_port = htons(MYPORT);     /* short, network byte order */
    my_addr.sin_addr.s_addr = INADDR_ANY; /* auto-fill with my IP */
    bzero(&(my_addr.sin_zero), 8);        /* zero the rest of the struct */

    if (bind(sockfd, (struct sockaddr *)&my_addr, sizeof(struct sockaddr)) \
                                                                  == -1) {
        perror("bind");
        exit(1);
    }

    if (listen(sockfd, BACKLOG) == -1) {
        perror("listen");
        exit(1);
    }

    while(1) {  /* main accept() loop */
        sin_size = sizeof(struct sockaddr_in);
        if ((new_fd = accept(sockfd, (struct sockaddr *)&their_addr, \
                                                      &sin_size)) == -1) {
            perror("accept");
            continue;
        }
        printf("server: got connection from %s\n", \
                                           inet_ntoa(their_addr.sin_addr));
        read(new_fd, buff, sizeof(buff)); 

        int cmd=atoi(buff);
        printf("[dotcp] got cmd %d\n",cmd);

        switch ( cmd ) {
            case 0xa0: // 160 change to TV
                libcec_set_active_source(g_connection, g_config.deviceTypes.types[1]);            
            break;
            case 0xa1: //161 change to HDMI 1
                libcec_set_active_source(g_connection, g_config.deviceTypes.types[0]);            
            break;
            case 0xa2: //162 change to HMDI
                cecHdmi();
            break;

            case 0xa3: 
                libcec_send_keypress(g_connection,CECDEVICE_TV, CEC_USER_CONTROL_CODE_SELECT,1);            
                libcec_send_key_release(g_connection,CECDEVICE_TV,1);            
            break;

            case 0xb0: // 176 create  signal file
                file_ptr = fopen(FILE_NAME, "w");
                fclose(file_ptr);
            break;
            case 0xb1: // 177 delete  signal file
                remove(FILE_NAME);        
            break;
            default:

            sendkeypress(atoi(buff));
            break;
        }
        if (send(new_fd, buff, sizeof(buff), 0) == -1)
             perror("send");
        close(new_fd);
    }
}
void* doSomeThing(void *arg)
{
    int i = 0;
    cec_logical_address src;
//    pthread_t id = pthread_self();

    while(1) {
        src=libcec_get_active_source(g_connection);
        printf("tick= %d %d\n",i, (int) src);
        if( (access( FILE_NAME, F_OK ) == -1) && (src!=1) ) { // file does not exists
            printf("[doSomeThing] go back to HDMI\n");
            cecHdmi();
        }
        sleep(2);
        i++;
    }
    return NULL;
}

int main(void)
{

    pthread_t tid;
    
    setupuinput();
	setupcec();

//	signal(SIGINT, death);
//	signal(SIGTERM, death);
    pthread_create(&(tid), NULL, &doSomeThing, NULL);

	while (!die)
		dotcp();

	cleancec();
	cleanuinput();

	return 0;
}
