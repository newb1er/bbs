#include "stdlib.h"
#include "base64.cpp"

struct chatmsg {
    unsigned char flag;
    unsigned char ver;
    unsigned char payload[0];
} __attribute__((packed));

struct msg_v2 {
    unsigned char data[0];
} __attribute__((packed));

struct msg_v1 {
    uint16_t len;
    unsigned char data[0];
} __attribute__((packed));


namespace Chat {
    std::list<std::pair<std::string, std::string>> history;

    const std::string filtering_list[] = {
        "how", "you", "or", "pek0", "tea", "ha", "kon", "pain", "Starburst Stream"
    };

    void AddHistory(std::string user, std::string msg) {
        Chat::history.push_back({user, msg});
    };

    bool filter(std::string &msg) {
        bool violate = false;

        for (std::string s : Chat::filtering_list) {
            if (msg.size() < s.size()) continue;
            size_t pos = msg.find(s);
            if (pos != std::string::npos) {
                violate = true;

                msg.replace(pos, s.size(), s.size(), '*');
            }
        }

        return violate;
    }

    bool ParseMessage(unsigned char *buf, std::string &username, bool &violate,
        std::array<std::pair<unsigned char *, size_t>, 2> &outbuf) {
        unsigned short len;
        struct msg_v1 *data_v1;
        struct msg_v2 *data_v2;
        std::string name = "", msg = "";
        unsigned char *str;
        unsigned char newbuf[4096];
        bzero(&newbuf, 4096);

        struct chatmsg *packet = (struct chatmsg *) buf;
        struct chatmsg *new_packet = (struct chatmsg *) newbuf;

        if (packet->flag != 0x1 && (packet->ver != 0x2 || packet->ver != 0x1))
            return false;


        if (packet->ver == 0x1) {
            outbuf[0].first = buf;

            // name
            data_v1 = (struct msg_v1 *) (packet->payload);
            len = ntohs(data_v1->len);
            str = new unsigned char[len];
            memcpy(str, data_v1->data, len);
            name.assign((char*) str, len);
            username = name;

            outbuf[0].second = len;
            memset(str, 0, len);

            // message
            data_v1 = (struct msg_v1 *) (data_v1->data + len);
            len = ntohs(data_v1->len);
            memcpy(str, data_v1->data, len);
            msg.assign((char*) str, len);

            if ((violate = filter(msg))) {
                memcpy(data_v1->data, msg.c_str(), msg.size());
            }

            Chat::history.push_back({name, msg});
            delete[] str;

            outbuf[0].second += len + 6; 

            // create v2 packet
            new_packet->flag = 0x1;
            new_packet->ver = 0x2;

            name = base64_encode(name) + "\n";
            msg = base64_encode(msg) + "\n";
            std::string payload = name + msg;

            memcpy(new_packet->payload, payload.c_str(), payload.size());
            
            outbuf[1].first = newbuf;
            outbuf[1].second = 2 + payload.size();
        }
        else {
            outbuf[1].first = buf;

            //name
            data_v2 = (struct msg_v2 *) packet->payload;
            len = strchr((char *) data_v2->data, '\n') - (char *) data_v2->data;
            str = new unsigned char[len];
            memcpy(str, data_v2->data, len);
            name.assign((char *) str, strlen((char *) str));
            name = base64_decode(name);
            username = name;

            memset(str, 0, len);
            outbuf[1].second = len;

            data_v2 = (struct msg_v2 *) (strchr((char *) data_v2->data, '\n') + 1);

            len = strchr((char *) data_v2->data, '\n') - (char *) data_v2->data;
            str = new unsigned char[len];
            memcpy(str, data_v2->data, len);
            msg.assign((char *) str, strlen((char *) str));
            msg = base64_decode(msg);

            if ((violate = filter(msg))) {
                auto encoded = base64_encode(msg);
                memcpy(data_v2->data, encoded.c_str(), encoded.size());
            }

            delete[] str;
            Chat::history.push_back({name, msg});

            outbuf[1].second += len + 4;

            // create v1 packet
            new_packet->flag = 0x1;
            new_packet->ver = 0x1;

            data_v1 = (struct msg_v1 *) new_packet->payload;
            data_v1->len = htons(name.size());
            memcpy(data_v1->data, name.c_str(), name.size());

            data_v1 = (struct msg_v1 *) (data_v1->data + name.size());

            data_v1->len = htons(msg.size());
            memcpy(data_v1->data, msg.c_str(), msg.size());

            outbuf[0].first = newbuf;
            outbuf[0].second = 6 + name.size() + msg.size();
        }
        return true;
    }
};