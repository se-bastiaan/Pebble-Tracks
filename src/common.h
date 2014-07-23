typedef struct {
	char code[6];
	char short_name[10];
	char full_name[25];
} Station;

typedef struct {
	char dep_time[5];
	char delay[10];
	char destination[25];
	char train_type[12];
	char route[256];
	char transporter[64];
	char track[3];
	bool track_changed;
	char tip[256];
	int index;
} Departure;

enum {
	REFRESH = 0x0,
	DATA = 0x1,
	ERROR = 0x2,
	CODE = 0x3,
	SHORT_NAME = 0x4,
	FULL_NAME = 0x5,
	TIME = 0x6,
	DELAY_TEXT = 0x7,
	DESTINATION = 0x8,
	TRAIN_TYPE = 0x9,
	ROUTE = 0xA,
	TRANSPORTER = 0xB,
	TRACK = 0xC,
	TRACK_CHANGED = 0xD,
	TIP = 0xE
};