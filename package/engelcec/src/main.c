#include <sys/stat.h>

#include <linux/input.h>
#include <linux/uinput.h>

#include <err.h>
#include <signal.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

#include <libcec/cecc.h>

#define LEN(x) (sizeof(x) / sizeof(*(x)))

#define DEBUG

#ifdef DEBUG
#define DPRINTF_X(t) printf(#t"=0x%x\n", t)
#define DPRINTF_D(t) printf(#t"=%d\n", t)
#define DPRINTF_U(t) printf(#t"=%u\n", t)
#define DPRINTF_S(t) printf(#t"=%s\n", t)
#define DPRINTF printf
#else
#define DPRINTF_X(t)
#define DPRINTF_D(t)
#define DPRINTF_U(t)
#define DPRINTF_S(t)
#define DPRINTF(t)
#endif

struct map {
	unsigned cec;  /* HDMI-CEC code */
	unsigned code; /* uinput code */
};

#include "config.h"

int fd;
int die;

void
setupuinput(void)
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

void
cleanuinput(void)
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
int onkeypress(void *cbparam, const cec_keypress key)
{
	static unsigned prevcode;

	DPRINTF_X(key.keycode);
	DPRINTF_X(key.duration);

	/* No duration means this is a press event.  Duration is set on
	 * release events.  Repeated press events are emulated by always
	 * sending a release event to uinput.  Some key presses only
	 * generate a single release event (the select button for example),
	 * so we inject a key press there by checking with the previous key
	 * press. */
	if (key.duration == 0 || key.keycode != prevcode)
		sendkeypress(key.keycode);

	prevcode = key.keycode;

	return 0;
}


int onlogmsg(void *cbparam, const cec_log_message msg)
{
	DPRINTF_S(msg.message);

	return 0;
}
int oncommand(void *cbparam, const cec_command cmd)
{
	return 0;
}

int onalert(void *cbparam, const libcec_alert type, const libcec_parameter param)
{
	return 0;
}

ICECCallbacks callbacks = {
	.CBCecKeyPress = onkeypress,
	.CBCecLogMessage = onlogmsg,
	.CBCecCommand = oncommand,
	.CBCecAlert = onalert,
};

libcec_configuration config = {
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

/* global cec connection handle */
libcec_connection_t conn;

void setupcec(void)
{
	cec_adapter devs[10], *dev;
	int ret;
	int n;

	conn = libcec_initialise(&config);
	if (!conn)
		errx(1, "cec init");

	/* Initialize host stack */
	libcec_init_video_standalone(conn);

	/* Auto detect adapters */
	n = libcec_find_adapters(conn, devs, LEN(devs), NULL);
	if (n == -1)
		errx(1, "find adapters");
	if (n == 0)
		errx(1, "no adapters found");
	DPRINTF_D(n);

	/* Take the first */
	dev = &devs[0];
	DPRINTF_S(dev->path);
	DPRINTF_S(dev->comm);

	ret = libcec_open(conn, dev->comm, CEC_DEFAULT_CONNECT_TIMEOUT);
	if (!ret)
		errx(1, "open port %s", dev->comm);
}

void cleancec(void)
{
	libcec_close(conn);
	libcec_destroy(conn);
}

void death(int sig)
{
	die = 1;
}

int main(void)
{
	setupuinput();
	setupcec();

	signal(SIGINT, death);
	signal(SIGTERM, death);

	while (!die)
		pause();

	cleancec();
	cleanuinput();

	return 0;
}
