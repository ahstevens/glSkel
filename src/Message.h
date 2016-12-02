#pragma once

enum MessageID {
	TEST
};

class Message
{
public:
	virtual ~Message() {}

	MessageID m_ID;
	void* m_pPayload;
};

