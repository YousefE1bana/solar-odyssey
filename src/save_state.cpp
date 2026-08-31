#include "save_state.h"
#include "solar_ui.h"
#include <fstream>
#include <sstream>
#include <iomanip>
#include <chrono>
#include <ctime>
#include <cctype>
#include <map>
#include <algorithm>
#include <iostream>

namespace json_mini {

enum Type { J_NULL, J_BOOL, J_NUM, J_STR, J_ARR, J_OBJ };

struct Value {
    Type type = J_NULL;
    bool bVal = false;
    double numVal = 0.0;
    std::string strVal;
    std::vector<Value> arrVal;
    std::map<std::string, Value> objVal;

    bool has(const std::string& key) const {
        return type == J_OBJ && objVal.find(key) != objVal.end();
    }

    const Value& get(const std::string& key) const {
        static Value sNull;
        if (type != J_OBJ) return sNull;
        auto it = objVal.find(key);
        return it != objVal.end() ? it->second : sNull;
    }

    double getNum(double def = 0.0) const {
        return type == J_NUM ? numVal : def;
    }

    float getFloat(float def = 0.0f) const {
        return type == J_NUM ? static_cast<float>(numVal) : def;
    }

    int getInt(int def = 0) const {
        return type == J_NUM ? static_cast<int>(numVal) : def;
    }

    bool getBool(bool def = false) const {
        return type == J_BOOL ? bVal : (type == J_NUM ? (numVal != 0) : def);
    }

    std::string getStr(const std::string& def = "") const {
        return type == J_STR ? strVal : def;
    }
};

class Parser {
public:
    static Value parse(const std::string& src, bool* outOk = nullptr) {
        bool ok = true;
        size_t idx = 0;
        skipWhitespace(src, idx);
        if (idx >= src.size()) {
            if (outOk) *outOk = false;
            return Value{};
        }
        Value v = parseValue(src, idx, ok);
        skipWhitespace(src, idx);
        if (idx < src.size()) ok = false;
        if (outOk) *outOk = ok;
        return ok ? v : Value{};
    }

private:
    static void skipWhitespace(const std::string& s, size_t& i) {
        while (i < s.size()) {
            if (std::isspace(static_cast<unsigned char>(s[i]))) {
                i++;
            } else if (s[i] == '/' && i + 1 < s.size() && s[i + 1] == '/') {
                // Line comment
                i += 2;
                while (i < s.size() && s[i] != '\n') i++;
            } else {
                break;
            }
        }
    }

    static Value parseValue(const std::string& s, size_t& i, bool& ok) {
        skipWhitespace(s, i);
        if (i >= s.size()) { ok = false; return Value{}; }

        char c = s[i];
        if (c == '{') return parseObject(s, i, ok);
        if (c == '[') return parseArray(s, i, ok);
        if (c == '"') return parseString(s, i, ok);
        if (c == 't' || c == 'f') return parseBool(s, i, ok);
        if (c == 'n') return parseNull(s, i, ok);
        if (c == '-' || std::isdigit(static_cast<unsigned char>(c))) return parseNumber(s, i, ok);

        ok = false;
        return Value{};
    }

    static Value parseObject(const std::string& s, size_t& i, bool& ok) {
        Value val;
        val.type = J_OBJ;
        i++; // skip '{'

        while (i < s.size()) {
            skipWhitespace(s, i);
            if (i >= s.size()) {
                ok = false;
                break;
            }
            if (s[i] == '}') {
                i++; // skip '}'
                return val;
            }

            if (s[i] != '"') {
                ok = false;
                return Value{};
            }

            Value keyVal = parseString(s, i, ok);
            if (!ok) return Value{};

            skipWhitespace(s, i);
            if (i >= s.size() || s[i] != ':') {
                ok = false;
                return Value{};
            }
            i++; // skip ':'

            Value itemVal = parseValue(s, i, ok);
            if (!ok) return Value{};
            val.objVal[keyVal.strVal] = itemVal;

            skipWhitespace(s, i);
            if (i < s.size() && s[i] == ',') {
                i++; // skip ','
            } else if (i < s.size() && s[i] == '}') {
                i++; // skip '}'
                return val;
            } else {
                ok = false;
                return Value{};
            }
        }
        ok = false;
        return Value{};
    }

    static Value parseArray(const std::string& s, size_t& i, bool& ok) {
        Value val;
        val.type = J_ARR;
        i++; // skip '['

        while (i < s.size()) {
            skipWhitespace(s, i);
            if (i >= s.size()) {
                ok = false;
                break;
            }
            if (s[i] == ']') {
                i++; // skip ']'
                return val;
            }

            Value itemVal = parseValue(s, i, ok);
            if (!ok) return Value{};
            val.arrVal.push_back(itemVal);

            skipWhitespace(s, i);
            if (i < s.size() && s[i] == ',') {
                i++; // skip ','
            } else if (i < s.size() && s[i] == ']') {
                i++; // skip ']'
                return val;
            } else {
                ok = false;
                return Value{};
            }
        }
        ok = false;
        return Value{};
    }

    static Value parseString(const std::string& s, size_t& i, bool& ok) {
        Value val;
        val.type = J_STR;
        i++; // skip opening '"'
        std::string str;

        while (i < s.size()) {
            char c = s[i++];
            if (c == '"') {
                val.strVal = str;
                return val;
            }
            if (c == '\\' && i < s.size()) {
                char esc = s[i++];
                if (esc == '"') str += '"';
                else if (esc == '\\') str += '\\';
                else if (esc == '/') str += '/';
                else if (esc == 'b') str += '\b';
                else if (esc == 'f') str += '\f';
                else if (esc == 'n') str += '\n';
                else if (esc == 'r') str += '\r';
                else if (esc == 't') str += '\t';
                else str += esc;
            } else {
                str += c;
            }
        }
        ok = false;
        return Value{};
    }

    static Value parseNumber(const std::string& s, size_t& i, bool& ok) {
        Value val;
        val.type = J_NUM;
        size_t start = i;

        if (s[i] == '-') i++;
        while (i < s.size() && std::isdigit(static_cast<unsigned char>(s[i]))) i++;
        if (i < s.size() && s[i] == '.') {
            i++;
            while (i < s.size() && std::isdigit(static_cast<unsigned char>(s[i]))) i++;
        }
        if (i < s.size() && (s[i] == 'e' || s[i] == 'E')) {
            i++;
            if (i < s.size() && (s[i] == '+' || s[i] == '-')) i++;
            while (i < s.size() && std::isdigit(static_cast<unsigned char>(s[i]))) i++;
        }

        std::string sub = s.substr(start, i - start);
        try {
            val.numVal = std::stod(sub);
        } catch (...) {
            ok = false;
        }
        return val;
    }

    static Value parseBool(const std::string& s, size_t& i, bool& ok) {
        Value val;
        val.type = J_BOOL;
        if (s.compare(i, 4, "true") == 0) {
            val.bVal = true;
            i += 4;
        } else if (s.compare(i, 5, "false") == 0) {
            val.bVal = false;
            i += 5;
        } else {
            ok = false;
        }
        return val;
    }

    static Value parseNull(const std::string& s, size_t& i, bool& ok) {
        Value val;
        val.type = J_NULL;
        if (s.compare(i, 4, "null") == 0) {
            i += 4;
        } else {
            ok = false;
        }
        return val;
    }
};

} // namespace json_mini

std::string SimulationSaveState::toJSON() const {
    std::ostringstream ss;
    ss << std::fixed << std::setprecision(8);

    ss << "{\n";
    ss << "  \"version\": " << version << ",\n";
    ss << "  \"timestamp\": \"" << timestamp << "\",\n";

    // Simulation block
    ss << "  \"simulation\": {\n";
    ss << "    \"elapsedSimDays\": " << elapsedSimDays << ",\n";
    ss << "    \"timeMultiplier\": " << timeMultiplier << ",\n";
    ss << "    \"isPaused\": " << (isPaused ? "true" : "false") << ",\n";
    ss << "    \"physicsMode\": " << physicsMode << "\n";
    ss << "  },\n";

    // Camera block
    ss << "  \"camera\": {\n";
    ss << "    \"mode\": " << camera.mode << ",\n";
    ss << "    \"eye\": [" << camera.eye.x << ", " << camera.eye.y << ", " << camera.eye.z << "],\n";
    ss << "    \"target\": [" << camera.target.x << ", " << camera.target.y << ", " << camera.target.z << "],\n";
    ss << "    \"up\": [" << camera.up.x << ", " << camera.up.y << ", " << camera.up.z << "],\n";
    ss << "    \"orbitDistance\": " << camera.orbitDistance << ",\n";
    ss << "    \"orbitAngleX\": " << camera.orbitAngleX << ",\n";
    ss << "    \"orbitAngleY\": " << camera.orbitAngleY << ",\n";
    ss << "    \"focusedBodyName\": \"" << camera.focusedBodyName << "\",\n";
    ss << "    \"focusDistance\": " << camera.focusDistance << ",\n";
    ss << "    \"focusAngleX\": " << camera.focusAngleX << ",\n";
    ss << "    \"focusAngleY\": " << camera.focusAngleY << ",\n";
    ss << "    \"freePos\": [" << camera.freePos.x << ", " << camera.freePos.y << ", " << camera.freePos.z << "],\n";
    ss << "    \"freeYaw\": " << camera.freeYaw << ",\n";
    ss << "    \"freePitch\": " << camera.freePitch << ",\n";
    ss << "    \"fov\": " << camera.fov << "\n";
    ss << "  },\n";

    // Missions block
    ss << "  \"missions\": {\n";
    ss << "    \"activeMissionIndex\": " << activeMissionIndex << ",\n";
    ss << "    \"missionsList\": [\n";
    for (size_t i = 0; i < missions.size(); ++i) {
        const auto& m = missions[i];
        ss << "      {\n";
        ss << "        \"id\": " << m.id << ",\n";
        ss << "        \"isCompleted\": " << (m.isCompleted ? "true" : "false") << ",\n";
        ss << "        \"progress\": " << m.progress << ",\n";
        ss << "        \"currentWaypointIndex\": " << m.currentWaypointIndex << "\n";
        ss << "      }" << (i + 1 < missions.size() ? "," : "") << "\n";
    }
    ss << "    ]\n";
    ss << "  },\n";

    // Spaceship block
    ss << "  \"spaceship\": {\n";
    ss << "    \"active\": " << (shipActive ? "true" : "false") << ",\n";
    ss << "    \"position\": [" << shipPosition.x << ", " << shipPosition.y << ", " << shipPosition.z << "],\n";
    ss << "    \"velocity\": [" << shipVelocity.x << ", " << shipVelocity.y << ", " << shipVelocity.z << "],\n";
    ss << "    \"throttle\": " << shipThrottle << ",\n";
    ss << "    \"targetBody\": \"" << shipTargetBody << "\"\n";
    ss << "  },\n";

    // Settings
    ss << "  \"settings\": {\n";
    ss << "    \"autoSaveOnExit\": " << (autoSaveOnExit ? "true" : "false") << "\n";
    ss << "  }\n";
    ss << "}\n";

    return ss.str();
}

bool SimulationSaveState::fromJSON(const std::string& jsonStr) {
    if (jsonStr.empty()) return false;

    bool parseOk = false;
    json_mini::Value root = json_mini::Parser::parse(jsonStr, &parseOk);
    if (!parseOk || root.type != json_mini::J_OBJ || root.objVal.empty()) return false;

    version = root.get("version").getInt(1);
    timestamp = root.get("timestamp").getStr("");

    // Simulation
    if (root.has("simulation")) {
        const auto& sim = root.get("simulation");
        elapsedSimDays = sim.get("elapsedSimDays").getNum(0.0);
        timeMultiplier = sim.get("timeMultiplier").getNum(1.0);
        isPaused = sim.get("isPaused").getBool(false);
        physicsMode = sim.get("physicsMode").getInt(0);
    }

    // Camera
    if (root.has("camera")) {
        const auto& cam = root.get("camera");
        camera.mode = cam.get("mode").getInt(0);

        if (cam.has("eye") && cam.get("eye").arrVal.size() >= 3) {
            camera.eye = glm::dvec3(cam.get("eye").arrVal[0].getNum(0.0),
                                    cam.get("eye").arrVal[1].getNum(35.0),
                                    cam.get("eye").arrVal[2].getNum(50.0));
        }
        if (cam.has("target") && cam.get("target").arrVal.size() >= 3) {
            camera.target = glm::dvec3(cam.get("target").arrVal[0].getNum(0.0),
                                       cam.get("target").arrVal[1].getNum(0.0),
                                       cam.get("target").arrVal[2].getNum(0.0));
        }
        if (cam.has("up") && cam.get("up").arrVal.size() >= 3) {
            camera.up = glm::dvec3(cam.get("up").arrVal[0].getNum(0.0),
                                   cam.get("up").arrVal[1].getNum(1.0),
                                   cam.get("up").arrVal[2].getNum(0.0));
        }

        camera.orbitDistance = cam.get("orbitDistance").getNum(50.0);
        camera.orbitAngleX = cam.get("orbitAngleX").getNum(0.0);
        camera.orbitAngleY = cam.get("orbitAngleY").getNum(60.0);
        camera.focusedBodyName = cam.get("focusedBodyName").getStr("Sun");
        camera.focusDistance = cam.get("focusDistance").getNum(8.0);
        camera.focusAngleX = cam.get("focusAngleX").getNum(45.0);
        camera.focusAngleY = cam.get("focusAngleY").getNum(70.0);

        if (cam.has("freePos") && cam.get("freePos").arrVal.size() >= 3) {
            camera.freePos = glm::dvec3(cam.get("freePos").arrVal[0].getNum(0.0),
                                        cam.get("freePos").arrVal[1].getNum(15.0),
                                        cam.get("freePos").arrVal[2].getNum(50.0));
        }
        camera.freeYaw = cam.get("freeYaw").getNum(-90.0);
        camera.freePitch = cam.get("freePitch").getNum(-15.0);
        camera.fov = cam.get("fov").getNum(60.0);
    }

    // Missions
    if (root.has("missions")) {
        const auto& mObj = root.get("missions");
        activeMissionIndex = mObj.get("activeMissionIndex").getInt(0);
        missions.clear();

        if (mObj.has("missionsList")) {
            for (const auto& mVal : mObj.get("missionsList").arrVal) {
                MissionSaveData mData;
                mData.id = mVal.get("id").getInt(0);
                mData.isCompleted = mVal.get("isCompleted").getBool(false);
                mData.progress = mVal.get("progress").getFloat(0.0f);
                mData.currentWaypointIndex = mVal.get("currentWaypointIndex").getInt(0);
                missions.push_back(mData);
            }
        }
    }

    // Spaceship
    if (root.has("spaceship")) {
        const auto& sObj = root.get("spaceship");
        shipActive = sObj.get("active").getBool(false);
        if (sObj.has("position") && sObj.get("position").arrVal.size() >= 3) {
            shipPosition = glm::dvec3(sObj.get("position").arrVal[0].getNum(0.0),
                                      sObj.get("position").arrVal[1].getNum(0.0),
                                      sObj.get("position").arrVal[2].getNum(0.0));
        }
        if (sObj.has("velocity") && sObj.get("velocity").arrVal.size() >= 3) {
            shipVelocity = glm::dvec3(sObj.get("velocity").arrVal[0].getNum(0.0),
                                      sObj.get("velocity").arrVal[1].getNum(0.0),
                                      sObj.get("velocity").arrVal[2].getNum(0.0));
        }
        shipThrottle = sObj.get("throttle").getNum(0.0);
        shipTargetBody = sObj.get("targetBody").getStr("Earth");
    }

    // Settings
    if (root.has("settings")) {
        autoSaveOnExit = root.get("settings").get("autoSaveOnExit").getBool(true);
    }

    return true;
}

SaveStateManager& SaveStateManager::instance() {
    static SaveStateManager mgr;
    return mgr;
}

bool SaveStateManager::saveToFile(const std::string& filepath, const SimulationSaveState& state) {
    std::ofstream file(filepath);
    if (!file.is_open()) return false;
    file << state.toJSON();
    return true;
}

bool SaveStateManager::loadFromFile(const std::string& filepath, SimulationSaveState& outState) {
    std::ifstream file(filepath);
    if (!file.is_open()) return false;
    std::stringstream ss;
    ss << file.rdbuf();
    return outState.fromJSON(ss.str());
}

bool SaveStateManager::fileExists(const std::string& filepath) const {
    std::ifstream f(filepath);
    return f.good();
}

void SaveStateManager::captureState(SimulationSaveState& outState,
                                   double elapsedSimDays, double timeMultiplier, bool isPaused, int physicsMode,
                                   const CameraController& cam, const MissionSystem& missions,
                                   const Spaceship& ship, bool autoSaveOnExit) {
    outState.version = 2;

    // ISO timestamp
    auto now = std::chrono::system_clock::now();
    std::time_t nowTime = std::chrono::system_clock::to_time_t(now);
    char buf[64];
    std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", std::localtime(&nowTime));
    outState.timestamp = std::string(buf);

    outState.elapsedSimDays = elapsedSimDays;
    outState.timeMultiplier = timeMultiplier;
    outState.isPaused = isPaused;
    outState.physicsMode = physicsMode;

    // Camera bookmark
    outState.camera.mode = static_cast<int>(cam.mode);
    outState.camera.eye = glm::dvec3(cam.currentEye);
    outState.camera.target = glm::dvec3(cam.currentTarget);
    outState.camera.up = glm::dvec3(cam.currentUp);
    outState.camera.orbitDistance = static_cast<double>(cam.orbitDistance);
    outState.camera.orbitAngleX = static_cast<double>(cam.orbitAngleX);
    outState.camera.orbitAngleY = static_cast<double>(cam.orbitAngleY);
    outState.camera.focusedBodyName = cam.focusedBodyName;
    outState.camera.focusDistance = static_cast<double>(cam.focusDistance);
    outState.camera.focusAngleX = static_cast<double>(cam.focusAngleX);
    outState.camera.focusAngleY = static_cast<double>(cam.focusAngleY);
    outState.camera.freePos = glm::dvec3(cam.freePos);
    outState.camera.freeYaw = static_cast<double>(cam.freeYaw);
    outState.camera.freePitch = static_cast<double>(cam.freePitch);
    outState.camera.fov = static_cast<double>(cam.fieldOfView);

    // Missions
    outState.activeMissionIndex = missions.activeMissionIndex;
    outState.missions.clear();
    for (const auto& m : missions.missions) {
        MissionSaveData md;
        md.id = m.id;
        md.isCompleted = m.isCompleted;
        md.progress = m.progress;
        md.currentWaypointIndex = m.currentWaypointIndex;
        outState.missions.push_back(md);
    }

    // Spaceship
    outState.shipActive = ship.active;
    outState.shipPosition = glm::dvec3(ship.position);
    outState.shipVelocity = glm::dvec3(ship.velocity);
    outState.shipThrottle = static_cast<double>(ship.throttle);
    outState.shipTargetBody = ship.targetPlanetName;

    outState.autoSaveOnExit = autoSaveOnExit;
}

void SaveStateManager::restoreState(const SimulationSaveState& state,
                                   float& outSimDays, float& outTimeMultiplier, bool& outPaused, int& outPhysicsMode,
                                   CameraController& cam, MissionSystem& missions,
                                   Spaceship& ship, SolarOdysseyUI& ui) {
    outSimDays = static_cast<float>(state.elapsedSimDays);
    outTimeMultiplier = static_cast<float>(state.timeMultiplier);
    outPaused = state.isPaused;
    outPhysicsMode = state.physicsMode;

    // Camera restoration
    cam.mode = static_cast<CameraMode>(state.camera.mode);
    cam.currentEye = glm::vec3(state.camera.eye);
    cam.currentTarget = glm::vec3(state.camera.target);
    cam.currentUp = glm::vec3(state.camera.up);
    cam.orbitDistance = static_cast<float>(state.camera.orbitDistance);
    cam.orbitAngleX = static_cast<float>(state.camera.orbitAngleX);
    cam.orbitAngleY = static_cast<float>(state.camera.orbitAngleY);
    cam.focusedBodyName = state.camera.focusedBodyName;
    cam.focusDistance = static_cast<float>(state.camera.focusDistance);
    cam.focusAngleX = static_cast<float>(state.camera.focusAngleX);
    cam.focusAngleY = static_cast<float>(state.camera.focusAngleY);
    cam.freePos = glm::vec3(state.camera.freePos);
    cam.freeYaw = static_cast<float>(state.camera.freeYaw);
    cam.freePitch = static_cast<float>(state.camera.freePitch);
    cam.fieldOfView = static_cast<float>(state.camera.fov);
    cam.targetFieldOfView = static_cast<float>(state.camera.fov);

    // Missions restoration
    missions.activeMissionIndex = std::min(state.activeMissionIndex, (int)missions.missions.size() - 1);
    for (const auto& savedM : state.missions) {
        for (auto& m : missions.missions) {
            if (m.id == savedM.id) {
                m.isCompleted = savedM.isCompleted;
                m.progress = savedM.progress;
                m.currentWaypointIndex = savedM.currentWaypointIndex;
                break;
            }
        }
    }

    // Spaceship restoration
    ship.active = state.shipActive;
    if (ship.active) {
        ship.position = glm::vec3(state.shipPosition);
        ship.velocity = glm::vec3(state.shipVelocity);
        ship.throttle = static_cast<float>(state.shipThrottle);
        ship.targetPlanetName = state.shipTargetBody;
    }

    ui.elapsedSimDays = static_cast<float>(state.elapsedSimDays);
    ui.timeMultiplier = static_cast<float>(state.timeMultiplier);
    ui.isPaused = state.isPaused;
    ui.physicsMode = state.physicsMode;
    ui.autoSaveOnExit = state.autoSaveOnExit;
    ui.selectedPlanetName = (cam.mode == CAM_FOCUS || cam.mode == CAM_POV) ? cam.focusedBodyName : "";
}

std::string SaveStateManager::getSaveSummary(const std::string& filepath) const {
    SimulationSaveState state;
    if (!const_cast<SaveStateManager*>(this)->loadFromFile(filepath, state)) {
        return "No save file found";
    }
    std::ostringstream ss;
    ss << "Day " << std::fixed << std::setprecision(1) << state.elapsedSimDays
       << " (" << state.timeMultiplier << "x, "
       << (state.camera.mode == CAM_FOCUS ? state.camera.focusedBodyName + " Focus" :
           state.camera.mode == CAM_FREE ? "Free Flight" :
           state.camera.mode == CAM_SPACESHIP ? "Spaceship" : "Overview")
       << ")";
    return ss.str();
}
