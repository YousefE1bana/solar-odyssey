#include "safe_file_write.h"
#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#endif
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
#include <cmath>
#include <limits>
#include <locale>
#include <set>
#include <filesystem>
#include <atomic>
#include <utility>
#include <initializer_list>
#include "canonical_inventory.h"
#ifdef _WIN32
#include <windows.h>
#endif

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

    static Value parseValue(const std::string& s, size_t& i, bool& ok, int depth = 0) {
        skipWhitespace(s, i);
        if (i >= s.size() || depth > 32) { ok = false; return Value{}; }

        char c = s[i];
        if (c == '{') return parseObject(s, i, ok, depth);
        if (c == '[') return parseArray(s, i, ok, depth);
        if (c == '"') return parseString(s, i, ok);
        if (c == 't' || c == 'f') return parseBool(s, i, ok);
        if (c == 'n') return parseNull(s, i, ok);
        if (c == '-' || std::isdigit(static_cast<unsigned char>(c))) return parseNumber(s, i, ok);

        ok = false;
        return Value{};
    }

    static Value parseObject(const std::string& s, size_t& i, bool& ok, int depth) {
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

            Value itemVal = parseValue(s, i, ok, depth + 1);
            if (!ok) return Value{};
            if (val.has(keyVal.strVal)) { ok = false; return Value{}; }
            val.objVal[keyVal.strVal] = itemVal;

            skipWhitespace(s, i);
            if (i < s.size() && s[i] == ',') {
                i++; // skip ','
                skipWhitespace(s, i);
                if (i >= s.size() || s[i] == '}') { ok = false; return Value{}; }
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

    static Value parseArray(const std::string& s, size_t& i, bool& ok, int depth) {
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

            Value itemVal = parseValue(s, i, ok, depth + 1);
            if (!ok) return Value{};
            val.arrVal.push_back(itemVal);

            skipWhitespace(s, i);
            if (i < s.size() && s[i] == ',') {
                i++; // skip ','
                skipWhitespace(s, i);
                if (i >= s.size() || s[i] == ']') { ok = false; return Value{}; }
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
                else if (esc == 'u') {
                    auto hex = [&](unsigned& code) {
                        code = 0;
                        for (int n = 0; n < 4; ++n) {
                            if (i >= s.size()) return false;
                            const char h = s[i++];
                            if (!std::isxdigit(static_cast<unsigned char>(h))) return false;
                            code = code * 16 + (h <= '9' ? h - '0' : std::tolower(static_cast<unsigned char>(h)) - 'a' + 10);
                        }
                        return true;
                    };
                    unsigned code = 0;
                    if (!hex(code)) { ok = false; return Value{}; }
                    if (code >= 0xd800 && code <= 0xdbff) {
                        if (i + 1 >= s.size() || s[i] != '\\' || s[i + 1] != 'u') { ok = false; return Value{}; }
                        i += 2;
                        unsigned low = 0;
                        if (!hex(low) || low < 0xdc00 || low > 0xdfff) { ok = false; return Value{}; }
                        code = 0x10000 + (code - 0xd800) * 1024 + low - 0xdc00;
                    } else if (code >= 0xdc00 && code <= 0xdfff) { ok = false; return Value{}; }
                    if (code <= 0x7f) str += static_cast<char>(code);
                    else {
                        if (code > 0xffff) str += static_cast<char>(0xf0 | (code >> 18));
                        if (code > 0x7ff) str += static_cast<char>((code > 0xffff ? 0x80 : 0xe0) | ((code >> 12) & 0x3f));
                        str += static_cast<char>((code > 0x7ff ? 0x80 : 0xc0) | ((code >> 6) & 0x3f));
                        str += static_cast<char>(0x80 | (code & 0x3f));
                    }
                } else { ok = false; return Value{}; }
            } else {
                if (static_cast<unsigned char>(c) < 0x20 || c == '\\') { ok = false; return Value{}; }
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
        const size_t integerStart = i;
        while (i < s.size() && std::isdigit(static_cast<unsigned char>(s[i]))) i++;
        if (i == integerStart || (s[integerStart] == '0' && i - integerStart > 1)) { ok = false; return Value{}; }
        if (i < s.size() && s[i] == '.') {
            i++;
            const size_t digits = i;
            while (i < s.size() && std::isdigit(static_cast<unsigned char>(s[i]))) i++;
            if (i == digits) { ok = false; return Value{}; }
        }
        if (i < s.size() && (s[i] == 'e' || s[i] == 'E')) {
            i++;
            if (i < s.size() && (s[i] == '+' || s[i] == '-')) i++;
            const size_t digits = i;
            while (i < s.size() && std::isdigit(static_cast<unsigned char>(s[i]))) i++;
            if (i == digits) { ok = false; return Value{}; }
        }

        std::string sub = s.substr(start, i - start);
        try {
            std::istringstream number(sub);
            number.imbue(std::locale::classic());
            if (!(number >> val.numVal) || !number.eof() || !std::isfinite(val.numVal)) ok = false;
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

namespace {
constexpr size_t kMaxSaveBytes = 16 * 1024 * 1024;
std::string escapeJSON(const std::string& input) {
    std::ostringstream out;
    for (unsigned char c : input) {
        if (c == '"' || c == '\\') out << '\\' << c;
        else if (c < 0x20) out << "\\u00" << std::hex << std::setw(2) << std::setfill('0') << unsigned(c);
        else out << c;
    }
    return out.str();
}
bool bounded(double v, double lo, double hi) { return std::isfinite(v) && v >= lo && v <= hi; }
bool vectorValid(const glm::dvec3& v) {
    return bounded(v.x, -1e12, 1e12) && bounded(v.y, -1e12, 1e12) && bounded(v.z, -1e12, 1e12);
}
bool textValid(const std::string& s, bool empty = false) {
    return (empty || !s.empty()) && s.size() <= 256 &&
           std::none_of(s.begin(), s.end(), [](unsigned char c) { return c < 0x20; });
}
bool bodyExists(const std::string& name) {
    if (name == "Sun") return true;
    for (const auto& b : CanonicalInventory::getCanonicalPlanets()) if (b.name == name) return true;
    for (const auto& b : CanonicalInventory::getCanonicalMoons()) if (b.name == name) return true;
    return false;
}
bool navigationTargetExists(const std::string& name) {
    return bodyExists(name) || name == "Black Hole" || name == "Gargantua" ||
           name == "Wormhole" || name == "Einstein-Rosen Bridge";
}
using json_mini::Value;
using json_mini::J_NUM;
using json_mini::J_BOOL;
using json_mini::J_STR;
using json_mini::J_OBJ;
using json_mini::J_ARR;
bool fields(const Value& v, json_mini::Type type, std::initializer_list<const char*> keys) {
    for (const char* key : keys) if (v.has(key) && v.get(key).type != type) return false;
    return true;
}
bool integers(const Value& v, std::initializer_list<const char*> keys) {
    for (const char* key : keys) {
        if (!v.has(key)) continue;
        const auto& n = v.get(key);
        if (n.type != J_NUM || !bounded(n.numVal, 0.0, double(std::numeric_limits<int>::max())) || std::floor(n.numVal) != n.numVal) return false;
    }
    return true;
}
bool vectorField(const Value& v, const char* key, size_t size = 3) {
    if (!v.has(key)) return true;
    const auto& a = v.get(key);
    return a.type == J_ARR && a.arrVal.size() == size &&
           std::all_of(a.arrVal.begin(), a.arrVal.end(), [](const Value& x) { return x.type == J_NUM && std::isfinite(x.numVal); });
}
glm::dvec3 vectorValue(const Value& v, const char* key, glm::dvec3 fallback) {
    if (!v.has(key)) return fallback;
    const auto& a = v.get(key).arrVal;
    return {a[0].numVal, a[1].numVal, a[2].numVal};
}
bool structureValid(const Value& root, int version) {
    if (!fields(root, J_STR, {"timestamp"}) ||
        !fields(root, J_OBJ, {"simulation", "camera", "spaceship", "progression", "settings", "presentation"})) return false;
    const auto& sim = root.get("simulation");
    if (sim.type != J_OBJ || !fields(sim, J_NUM, {"elapsedSimDays", "simTimeSeconds", "timeMultiplier", "orbitSpeedScale", "cloudRotationAngle"}) ||
        !fields(sim, J_BOOL, {"isPaused"}) || !integers(sim, {"physicsMode"}) || !fields(sim, J_STR, {"timeUnit"}) ||
        !fields(sim, J_OBJ, {"numerical"})) return false;
    if (version < 4 && !sim.has("elapsedSimDays")) return false;
    const auto& cam = root.get("camera");
    if (!integers(cam, {"mode"}) || !fields(cam, J_STR, {"focusedBodyName"}) ||
        !fields(cam, J_NUM, {"orbitDistance", "orbitAngleX", "orbitAngleY", "focusDistance", "focusAngleX", "focusAngleY", "freeYaw", "freePitch", "freeSpeed", "fov", "povHeight", "povOrbitAngle", "minFocusDistance", "maxFocusDistance"})) return false;
    for (const char* key : {"eye", "target", "up", "freePos"}) if (!vectorField(cam, key)) return false;
    const auto& ship = root.get("spaceship");
    if (!fields(ship, J_BOOL, {"active"}) || !fields(ship, J_NUM, {"throttle", "boostEnergy"}) ||
        !fields(ship, J_STR, {"targetBody"}) || !integers(ship, {"cameraView"}) ||
        !vectorField(ship, "position") || !vectorField(ship, "velocity") || !vectorField(ship, "orientation", 4)) return false;
    if (!fields(root.get("settings"), J_BOOL, {"autoSaveOnExit"})) return false;
    const auto& presentation = root.get("presentation");
    if (!integers(presentation, {"mode"}) || !fields(presentation, J_STR, {"selectedBodyName"})) return false;
    const auto& prog = root.get("progression");
    if (!fields(prog, J_ARR, {"records"}) || prog.get("records").arrVal.size() > 1024) return false;
    std::set<std::string> targets;
    for (const auto& r : prog.get("records").arrVal) {
        if (r.type != J_OBJ || !r.has("target") || !fields(r, J_STR, {"target"}) ||
            !targets.insert(r.get("target").strVal).second || !integers(r, {"status", "visitCount"}) ||
            !fields(r, J_NUM, {"bestPhotoScore"}) || !fields(r, J_ARR, {"anomalies"}) ||
            !fields(r, J_BOOL, {"detected", "visited", "firstVisitRecorded", "atmosphereApplicable", "orbitalSurveyCompleted", "atmosphericScanCompleted", "gravityMeasurementCompleted", "closeFlybyCompleted"})) return false;
        if (r.has("bestPhotoScore") && !bounded(r.get("bestPhotoScore").numVal, 0.0, 100.0)) return false;
        if (version >= 4) {
            for (const char* key : {"target", "status", "detected", "visited", "firstVisitRecorded", "visitCount", "atmosphereApplicable", "orbitalSurveyCompleted", "atmosphericScanCompleted", "bestPhotoScore", "gravityMeasurementCompleted", "closeFlybyCompleted", "anomalies"}) if (!r.has(key)) return false;
        }
        for (const auto& a : r.get("anomalies").arrVal) if (a.type != J_STR) return false;
    }
    if (version >= 4) {
        if (version >= 5) {
            if (!sim.has("analyticEpochSeconds") || !fields(sim, J_NUM, {"analyticEpochSeconds"}) ||
                !sim.has("analyticOffsets") || !fields(sim, J_ARR, {"analyticOffsets"})) return false;
            std::set<std::string> names;
            for (const auto& entry : sim.get("analyticOffsets").arrVal) {
                if (entry.type != J_OBJ || !entry.has("name") || !fields(entry, J_STR, {"name"}) ||
                    !entry.has("offset") || !vectorField(entry, "offset") ||
                    !names.insert(entry.get("name").strVal).second) return false;
            }
        }
        for (const char* key : {"camera", "spaceship", "settings", "progression", "presentation"}) if (!root.has(key)) return false;
        for (const char* key : {"simTimeSeconds", "timeUnit", "timeMultiplier", "isPaused", "physicsMode", "orbitSpeedScale", "cloudRotationAngle", "numerical"}) if (!sim.has(key)) return false;
        if (sim.get("timeUnit").strVal != "simulation_seconds") return false;
        for (const char* key : {"mode", "eye", "target", "up", "orbitDistance", "orbitAngleX", "orbitAngleY", "focusedBodyName", "focusDistance", "focusAngleX", "focusAngleY", "freePos", "freeYaw", "freePitch", "freeSpeed", "fov", "povHeight", "povOrbitAngle", "minFocusDistance", "maxFocusDistance"}) if (!cam.has(key)) return false;
        for (const char* key : {"active", "position", "velocity", "throttle", "targetBody", "orientation", "boostEnergy", "cameraView"}) if (!ship.has(key)) return false;
        if (!presentation.has("mode") || !presentation.has("selectedBodyName") || !prog.has("records") ||
            !root.get("settings").has("autoSaveOnExit")) return false;
        const auto& n = sim.get("numerical");
        if (!fields(n, J_NUM, {"gravitationalConstant", "softening", "fixedDeltaTime", "timeAccumulator", "maxAccumulatorCap"}) || !fields(n, J_ARR, {"roots"})) return false;
        for (const char* key : {"gravitationalConstant", "softening", "fixedDeltaTime", "timeAccumulator", "maxAccumulatorCap", "roots"}) if (!n.has(key)) return false;
        for (const auto& r : n.get("roots").arrVal) {
            if (r.type != J_OBJ || !r.has("name") || !r.has("position") || !r.has("velocity") ||
                !fields(r, J_STR, {"name"}) || !vectorField(r, "position") || !vectorField(r, "velocity")) return false;
        }
    }
    return true;
}
} // namespace

bool SimulationSaveState::validate() const {
    const auto& c = camera;
    if (version < 1 || version > 5 || !textValid(timestamp, true) ||
        !bounded(simulationSeconds(), 0.0, 1e12) || !bounded(timeMultiplier, 0.0, 1e6) ||
        physicsMode < 0 || physicsMode > 1 || c.mode < CAM_ORBITAL || c.mode > CAM_WORMHOLE ||
        !vectorValid(c.eye) || !vectorValid(c.target) || !vectorValid(c.up) || !vectorValid(c.freePos) ||
        glm::length(c.up) < 1e-6 || glm::length(c.target - c.eye) < 1e-6 ||
        glm::length(glm::cross(glm::normalize(c.up), glm::normalize(c.target - c.eye))) < 1e-6 ||
        !bounded(c.orbitDistance, 3.0, 350.0) || !bounded(c.orbitAngleX, -1e12, 1e12) || !bounded(c.orbitAngleY, 1.0, 179.0) ||
        !bounded(c.focusDistance, 0.001, 1e6) || !bounded(c.focusAngleX, -1e12, 1e12) || !bounded(c.focusAngleY, 1.0, 179.0) ||
        !bounded(c.freeYaw, -1e12, 1e12) || !bounded(c.freePitch, -89.0, 89.0) || !bounded(c.freeSpeed, 2.0, 150.0) || !bounded(c.fov, 1.0, 179.0) ||
        !textValid(c.focusedBodyName, true) || !bounded(c.povHeight, 0.0, 1e6) || !bounded(c.povOrbitAngle, -1e12, 1e12) ||
        !bounded(c.minFocusDistance, 0.001, 1e6) || !bounded(c.maxFocusDistance, c.minFocusDistance, 1e6) ||
        !vectorValid(shipPosition) || !vectorValid(shipVelocity) || !bounded(shipThrottle, 0.0, 1.0) || !navigationTargetExists(shipTargetBody)) return false;
    const glm::vec3 floatDirection = glm::vec3(c.target) - glm::vec3(c.eye);
    if (glm::length(floatDirection) < 1e-6f ||
        glm::length(glm::cross(glm::normalize(glm::vec3(c.up)), glm::normalize(floatDirection))) < 1e-6f) return false;
    if ((c.mode == CAM_FOCUS || c.mode == CAM_POV || c.mode == CAM_TOUR) && !bodyExists(c.focusedBodyName)) return false;
    if (c.mode == CAM_BLACK_HOLE && c.focusedBodyName != "Black Hole" && c.focusedBodyName != "Gargantua") return false;
    if (c.mode == CAM_WORMHOLE && c.focusedBodyName != "Wormhole" && c.focusedBodyName != "Einstein-Rosen Bridge") return false;
    std::set<std::string> targets;
    if (progression.records.size() > 1024) return false;
    for (const auto& r : progression.records) {
        if (!textValid(r.target) || !targets.insert(r.target).second || r.status < 0 || r.status > 4 || r.visitCount < 0 ||
            !bounded(r.bestPhotoScore, 0.0, 100.0) || r.anomalyIds.size() > 1024) return false;
        std::set<std::string> anomalies;
        for (const auto& id : r.anomalyIds) if (!textValid(id) || !anomalies.insert(id).second) return false;
    }
    if (version >= 4) {
        const auto& n = continuation;
        if (version >= 5) {
            if (!bounded(n.analyticEpochSeconds, 0.0, simulationSeconds())) return false;
            std::set<std::string> expected;
            for (const auto& b : CanonicalInventory::getCanonicalPlanets()) expected.insert(b.name);
            for (const auto& b : CanonicalInventory::getCanonicalMoons()) expected.insert(b.name);
            if (!n.analyticOffsets.empty() && n.analyticOffsets.size() != expected.size()) return false;
            for (const auto& entry : n.analyticOffsets)
                if (!expected.erase(entry.first) || !vectorValid(entry.second) || glm::length(entry.second) < 1e-9) return false;
        }
        const double qLength = glm::length(shipOrientation);
        if (!bounded(qLength, 0.999, 1.001) || !bounded(shipBoostEnergy, 0.0, 100.0) || shipCameraView < 0 || shipCameraView > 2 ||
            presentationMode < 0 || presentationMode > 2 || !textValid(selectedBodyName, true) ||
            (!selectedBodyName.empty() && !navigationTargetExists(selectedBodyName)) ||
            (presentationMode != 0 && shipActive) ||
            (presentationMode == 2 && (selectedBodyName != c.focusedBodyName || c.mode != CAM_FOCUS)) ||
            !bounded(elapsedSimDays, 0.0, 5e12) || std::abs(elapsedSimDays - displayDays()) > 8.0 * std::numeric_limits<double>::epsilon() * std::max(1.0, displayDays()) ||
            !bounded(n.orbitSpeedScale, 0.0, 100.0) || !bounded(n.cloudRotationAngle, -1e12, 1e12) ||
            !bounded(n.gravitationalConstant, 1e-9, 1e9) || !bounded(n.softening, 1e-9, 1e6) ||
            !bounded(n.fixedDeltaTime, 1e-6, 1.0) || !bounded(n.maxAccumulatorCap, n.fixedDeltaTime, 10.0) ||
            !bounded(n.timeAccumulator, 0.0, n.fixedDeltaTime) || n.timeAccumulator >= n.fixedDeltaTime ||
            n.maxAccumulatorCap / n.fixedDeltaTime > 4096.0) return false;
        std::set<std::string> expected;
        for (const auto& b : CanonicalInventory::getCanonicalNBodyObjects()) if (b.parentPlanet.empty()) expected.insert(b.name);
        if (physicsMode == PHYSICS_NBODY) {
            if (n.roots.size() != expected.size()) return false;
            for (const auto& r : n.roots) {
                if (!expected.erase(r.name) || !vectorValid(r.position) || !vectorValid(r.velocity)) return false;
                if (r.name == "Sun" && (r.position != glm::dvec3(0.0) || r.velocity != glm::dvec3(0.0))) return false;
            }
        } else if (!n.roots.empty()) return false;
    }
    return true;
}

std::string SimulationSaveState::toJSON() const {
    std::ostringstream ss;
    ss.imbue(std::locale::classic());
    ss << std::setprecision(std::numeric_limits<double>::max_digits10);

    ss << "{\n";
    ss << "  \"version\": " << version << ",\n";
    ss << "  \"timestamp\": \"" << escapeJSON(timestamp) << "\",\n";

    // Simulation block
    ss << "  \"simulation\": {\n";
    if (version >= 4) {
        ss << "    \"timeUnit\": \"simulation_seconds\",\n";
        ss << "    \"simTimeSeconds\": " << simTimeSeconds << ",\n";
        ss << "    \"orbitSpeedScale\": " << continuation.orbitSpeedScale << ",\n";
        ss << "    \"cloudRotationAngle\": " << continuation.cloudRotationAngle << ",\n";
        if (version >= 5) {
            ss << "    \"analyticEpochSeconds\": " << continuation.analyticEpochSeconds << ",\n";
            ss << "    \"analyticOffsets\": [";
            bool first = true;
            for (const auto& entry : continuation.analyticOffsets) {
                const auto& p = entry.second;
                ss << (first ? "" : ",") << "{\"name\": \"" << escapeJSON(entry.first)
                   << "\", \"offset\": [" << p.x << ", " << p.y << ", " << p.z << "]}";
                first = false;
            }
            ss << "],\n";
        }
        ss << "    \"numerical\": {\n";
        ss << "      \"gravitationalConstant\": " << continuation.gravitationalConstant << ",\n";
        ss << "      \"softening\": " << continuation.softening << ",\n";
        ss << "      \"fixedDeltaTime\": " << continuation.fixedDeltaTime << ",\n";
        ss << "      \"timeAccumulator\": " << continuation.timeAccumulator << ",\n";
        ss << "      \"maxAccumulatorCap\": " << continuation.maxAccumulatorCap << ",\n";
        ss << "      \"roots\": [";
        for (size_t i = 0; i < continuation.roots.size(); ++i) {
            const auto& r = continuation.roots[i];
            ss << (i ? "," : "") << "\n        {\"name\": \"" << escapeJSON(r.name) << "\", \"position\": [" << r.position.x << ", " << r.position.y << ", " << r.position.z
               << "], \"velocity\": [" << r.velocity.x << ", " << r.velocity.y << ", " << r.velocity.z << "]}";
        }
        ss << "]\n    },\n";
    }
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
    ss << "    \"focusedBodyName\": \"" << escapeJSON(camera.focusedBodyName) << "\",\n";
    ss << "    \"focusDistance\": " << camera.focusDistance << ",\n";
    ss << "    \"focusAngleX\": " << camera.focusAngleX << ",\n";
    ss << "    \"focusAngleY\": " << camera.focusAngleY << ",\n";
    ss << "    \"freePos\": [" << camera.freePos.x << ", " << camera.freePos.y << ", " << camera.freePos.z << "],\n";
    ss << "    \"freeYaw\": " << camera.freeYaw << ",\n";
    ss << "    \"freePitch\": " << camera.freePitch << ",\n";
    if (version >= 4) {
        ss << "    \"freeSpeed\": " << camera.freeSpeed << ",\n";
        ss << "    \"povHeight\": " << camera.povHeight << ",\n";
        ss << "    \"povOrbitAngle\": " << camera.povOrbitAngle << ",\n";
        ss << "    \"minFocusDistance\": " << camera.minFocusDistance << ",\n";
        ss << "    \"maxFocusDistance\": " << camera.maxFocusDistance << ",\n";
    }
    ss << "    \"fov\": " << camera.fov << "\n";
    ss << "  },\n";

    // Spaceship block
    ss << "  \"spaceship\": {\n";
    ss << "    \"active\": " << (shipActive ? "true" : "false") << ",\n";
    ss << "    \"position\": [" << shipPosition.x << ", " << shipPosition.y << ", " << shipPosition.z << "],\n";
    ss << "    \"velocity\": [" << shipVelocity.x << ", " << shipVelocity.y << ", " << shipVelocity.z << "],\n";
    ss << "    \"throttle\": " << shipThrottle << ",\n";
    if (version >= 4) {
        ss << "    \"orientation\": [" << shipOrientation.w << ", " << shipOrientation.x << ", " << shipOrientation.y << ", " << shipOrientation.z << "],\n";
        ss << "    \"boostEnergy\": " << shipBoostEnergy << ",\n";
        ss << "    \"cameraView\": " << shipCameraView << ",\n";
    }
    ss << "    \"targetBody\": \"" << escapeJSON(shipTargetBody) << "\"\n";
    ss << "  },\n";

    if (version >= 4) {
        ss << "  \"presentation\": {\"mode\": " << presentationMode << ", \"selectedBodyName\": \"" << escapeJSON(selectedBodyName) << "\"},\n";
    }

    // Cycle 4 progression block (v3). Compact metadata only — never pixels.
    ss << "  \"progression\": {\n";
    ss << "    \"records\": [\n";
    for (size_t i = 0; i < progression.records.size(); ++i) {
        const auto& r = progression.records[i];
        ss << "      {\n";
        ss << "        \"target\": \"" << escapeJSON(r.target) << "\",\n";
        ss << "        \"status\": " << r.status << ",\n";
        ss << "        \"detected\": " << (r.detected ? "true" : "false") << ",\n";
        ss << "        \"visited\": " << (r.visited ? "true" : "false") << ",\n";
        ss << "        \"firstVisitRecorded\": " << (r.firstVisitRecorded ? "true" : "false") << ",\n";
        ss << "        \"visitCount\": " << r.visitCount << ",\n";
        ss << "        \"atmosphereApplicable\": " << (r.atmosphereApplicable ? "true" : "false") << ",\n";
        ss << "        \"orbitalSurveyCompleted\": " << (r.orbitalSurveyCompleted ? "true" : "false") << ",\n";
        ss << "        \"atmosphericScanCompleted\": " << (r.atmosphericScanCompleted ? "true" : "false") << ",\n";
        ss << "        \"bestPhotoScore\": " << r.bestPhotoScore << ",\n";
        ss << "        \"gravityMeasurementCompleted\": " << (r.gravityMeasurementCompleted ? "true" : "false") << ",\n";
        ss << "        \"closeFlybyCompleted\": " << (r.closeFlybyCompleted ? "true" : "false") << ",\n";
        ss << "        \"anomalies\": [";
        for (size_t j = 0; j < r.anomalyIds.size(); ++j) {
            ss << "\"" << escapeJSON(r.anomalyIds[j]) << "\"" << (j + 1 < r.anomalyIds.size() ? ", " : "");
        }
        ss << "]\n";
        ss << "      }" << (i + 1 < progression.records.size() ? "," : "") << "\n";
    }
    ss << "    ]\n";
    ss << "  },\n";

    // Settings
    ss << "  \"settings\": {\n";
    ss << "    \"autoSaveOnExit\": " << (autoSaveOnExit ? "true" : "false") << "\n";
    ss << "  }\n";
    ss << "}\n";

    return ss.str();
}

bool SimulationSaveState::fromJSON(const std::string& jsonStr) {
    if (jsonStr.empty() || jsonStr.size() > kMaxSaveBytes) return false;

    bool parseOk = false;
    json_mini::Value root = json_mini::Parser::parse(jsonStr, &parseOk);
    if (!parseOk || root.type != json_mini::J_OBJ || root.objVal.empty()) return false;

    if (!integers(root, {"version"})) return false;
    const int fileVersion = root.get("version").getInt(1);
    if (fileVersion < 1 || fileVersion > 5 || !structureValid(root, fileVersion)) return false;
    // Never partially overwrite the caller's state, even on a late failure.
    SimulationSaveState parsed;
    auto& version = parsed.version;
    auto& timestamp = parsed.timestamp;
    auto& elapsedSimDays = parsed.elapsedSimDays;
    auto& timeMultiplier = parsed.timeMultiplier;
    auto& isPaused = parsed.isPaused;
    auto& physicsMode = parsed.physicsMode;
    auto& camera = parsed.camera;
    auto& shipActive = parsed.shipActive;
    auto& shipPosition = parsed.shipPosition;
    auto& shipVelocity = parsed.shipVelocity;
    auto& shipThrottle = parsed.shipThrottle;
    auto& shipTargetBody = parsed.shipTargetBody;
    auto& progression = parsed.progression;
    auto& autoSaveOnExit = parsed.autoSaveOnExit;

    version = root.get("version").getInt(1);
    timestamp = root.get("timestamp").getStr("");

    // Simulation
    if (root.has("simulation")) {
        const auto& sim = root.get("simulation");
        elapsedSimDays = sim.get("elapsedSimDays").getNum(0.0);
        timeMultiplier = sim.get("timeMultiplier").getNum(1.0);
        isPaused = sim.get("isPaused").getBool(false);
        physicsMode = sim.get("physicsMode").getInt(0);
        if (version >= 4) {
            parsed.simTimeSeconds = sim.get("simTimeSeconds").getNum();
            // v4 displayed days are redundant and may be omitted; the unit
            // tagged simulation clock remains the sole time authority.
            elapsedSimDays = sim.get("elapsedSimDays").getNum(parsed.displayDays());
            auto& n = parsed.continuation;
            if (version >= 5) {
                n.analyticEpochSeconds = sim.get("analyticEpochSeconds").getNum();
                for (const auto& entry : sim.get("analyticOffsets").arrVal)
                    n.analyticOffsets.emplace(entry.get("name").getStr(), vectorValue(entry, "offset", {}));
            }
            n.orbitSpeedScale = sim.get("orbitSpeedScale").getNum();
            n.cloudRotationAngle = sim.get("cloudRotationAngle").getNum();
            const auto& numeric = sim.get("numerical");
            n.gravitationalConstant = numeric.get("gravitationalConstant").getNum();
            n.softening = numeric.get("softening").getNum();
            n.fixedDeltaTime = numeric.get("fixedDeltaTime").getNum();
            n.timeAccumulator = numeric.get("timeAccumulator").getNum();
            n.maxAccumulatorCap = numeric.get("maxAccumulatorCap").getNum();
            for (const auto& r : numeric.get("roots").arrVal)
                n.roots.push_back({r.get("name").getStr(), vectorValue(r, "position", {}), vectorValue(r, "velocity", {})});
        }
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
        camera.freeSpeed = cam.get("freeSpeed").getNum(20.0);
        if (camera.mode == CAM_FREE && !cam.has("freePos")) camera.freePos = camera.eye;
        if (camera.mode == CAM_FREE && (!cam.has("freeYaw") || !cam.has("freePitch"))) {
            const glm::dvec3 direction = camera.target - camera.eye;
            if (glm::length(direction) < 1e-6) return false;
            const auto front = glm::normalize(direction);
            camera.freeYaw = glm::degrees(std::atan2(front.z, front.x));
            camera.freePitch = glm::degrees(std::asin(std::max(-0.999, std::min(0.999, front.y))));
        }
        camera.fov = cam.get("fov").getNum(60.0);
        camera.povHeight = cam.get("povHeight").getNum(0.25);
        camera.povOrbitAngle = cam.get("povOrbitAngle").getNum(0.0);
        camera.minFocusDistance = cam.get("minFocusDistance").getNum(1.0);
        camera.maxFocusDistance = cam.get("maxFocusDistance").getNum(std::max(50.0, camera.focusDistance));
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
        if (version >= 4) {
            const auto& q = sObj.get("orientation").arrVal;
            parsed.shipOrientation = glm::dquat(q[0].numVal, q[1].numVal, q[2].numVal, q[3].numVal);
            parsed.shipBoostEnergy = sObj.get("boostEnergy").getNum();
            parsed.shipCameraView = sObj.get("cameraView").getInt();
        }
    }

    // Cycle 4 progression (v3 only). v2 saves carry no such block: the
    // default-fresh ProgressionSaveData member is kept untouched, so old
    // saves load with new-game progression and all v2 data preserved.
    if (version >= 3 && root.has("progression")) {
        const auto& pObj = root.get("progression");
        progression.records.clear();
        if (pObj.has("records")) {
            for (const auto& rVal : pObj.get("records").arrVal) {
                DiscoveryRecordSave r;
                r.target = rVal.get("target").getStr("");
                if (r.target.empty()) continue;
                r.status = rVal.get("status").getInt(0);
                r.detected = rVal.get("detected").getBool(false);
                r.visited = rVal.get("visited").getBool(false);
                r.firstVisitRecorded = rVal.get("firstVisitRecorded").getBool(false);
                r.visitCount = rVal.get("visitCount").getInt(0);
                r.atmosphereApplicable = rVal.get("atmosphereApplicable").getBool(false);
                r.orbitalSurveyCompleted = rVal.get("orbitalSurveyCompleted").getBool(false);
                r.atmosphericScanCompleted = rVal.get("atmosphericScanCompleted").getBool(false);
                r.bestPhotoScore = rVal.get("bestPhotoScore").getFloat(0.0f);
                r.gravityMeasurementCompleted = rVal.get("gravityMeasurementCompleted").getBool(false);
                r.closeFlybyCompleted = rVal.get("closeFlybyCompleted").getBool(false);
                if (rVal.has("anomalies")) {
                    for (const auto& aVal : rVal.get("anomalies").arrVal) {
                        const std::string aid = aVal.getStr("");
                        if (!aid.empty()) r.anomalyIds.push_back(aid);
                    }
                }
                progression.records.push_back(r);
            }
        }
    }

    // Settings
    if (root.has("settings")) {
        autoSaveOnExit = root.get("settings").get("autoSaveOnExit").getBool(true);
    }

    if (version >= 4) {
        parsed.presentationMode = root.get("presentation").get("mode").getInt();
        parsed.selectedBodyName = root.get("presentation").get("selectedBodyName").getStr();
    }
    if (!parsed.validate()) return false;
    *this = std::move(parsed);

    return true;
}

SaveStateManager& SaveStateManager::instance() {
    static SaveStateManager mgr;
    return mgr;
}

bool SaveStateManager::saveToFile(const std::string& filepath, const SimulationSaveState& state) {
    if (!state.validate()) return false;
    const std::string payload = state.toJSON();
    SimulationSaveState check;
    if (!check.fromJSON(payload)) return false;
    return safeWriteFile(std::filesystem::u8path(filepath), payload);
}

bool SaveStateManager::loadFromFile(const std::string& filepath, SimulationSaveState& outState) {
    std::ifstream file(std::filesystem::u8path(filepath), std::ios::binary | std::ios::ate);
    if (!file.is_open()) return false;
    const auto size = file.tellg();
    if (size <= 0 || size > static_cast<std::streamoff>(kMaxSaveBytes)) return false;
    std::string json(static_cast<size_t>(size), '\0');
    file.seekg(0);
    if (!file.read(json.data(), static_cast<std::streamsize>(json.size()))) return false;
    return outState.fromJSON(json);
}

bool SaveStateManager::fileExists(const std::string& filepath) const {
    std::ifstream f(filepath);
    return f.good();
}

void SaveStateManager::captureState(SimulationSaveState& outState,
                                   double legacySimSeconds, double timeMultiplier, bool isPaused, int physicsMode,
                                   const CameraController& cam,
                                   const Spaceship& ship, bool autoSaveOnExit) {
    outState = SimulationSaveState{};
    outState.version = 3;

    // ISO timestamp
    auto now = std::chrono::system_clock::now();
    std::time_t nowTime = std::chrono::system_clock::to_time_t(now);
    char buf[64];
    std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", std::localtime(&nowTime));
    outState.timestamp = std::string(buf);

    outState.elapsedSimDays = legacySimSeconds;
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

    // Spaceship
    outState.shipActive = ship.active;
    outState.shipPosition = glm::dvec3(ship.position);
    outState.shipVelocity = glm::dvec3(ship.velocity);
    outState.shipThrottle = static_cast<double>(ship.throttle);
    outState.shipTargetBody = ship.targetPlanetName;

    outState.autoSaveOnExit = autoSaveOnExit;
}

void SaveStateManager::captureProgression(SimulationSaveState& outState, const ScienceProgression& prog) {
    if (outState.version < 3) outState.version = 3;
    outState.progression = prog.captureSaveData();
}

void SaveStateManager::captureState(SimulationSaveState& out, const SimulationController& sim,
                                   const CameraController& cam, const Spaceship& ship, bool autoSave,
                                   int presentation, const std::string& selection) {
    out = SimulationSaveState{};
    captureState(out, sim.getSimTime(), sim.getTimeMultiplier(), sim.isPaused(), sim.getPhysicsMode(), cam, ship, autoSave);
    out.version = 5;
    out.simTimeSeconds = sim.getSimTime();
    out.elapsedSimDays = sim.getElapsedSimDays();
    out.continuation = sim.captureContinuation();
    out.camera.povHeight = cam.povHeight;
    out.camera.freeSpeed = cam.freeSpeed;
    out.camera.povOrbitAngle = cam.povOrbitAngle;
    out.camera.minFocusDistance = cam.minFocusDistance;
    out.camera.maxFocusDistance = cam.maxFocusDistance;
    out.shipOrientation = glm::dquat(ship.orientation);
    out.shipBoostEnergy = ship.boostEnergy;
    out.shipCameraView = ship.cameraView;
    // The saved lens is the base camera FOV; warp's projection offset and
    // photo session/history are deliberately absent from the bookmark.
    out.presentationMode = presentation;
    out.selectedBodyName = selection;
    if (!out.selectedBodyName.empty() && !navigationTargetExists(out.selectedBodyName)) out.selectedBodyName.clear();
    if (cam.mode == CAM_TRANSITION || cam.mode == CAM_TOUR) {
        CameraController stationary = cam;
        stationary.enterFreeCam();
        out.camera.mode = CAM_FREE;
        out.camera.freePos = glm::dvec3(stationary.freePos);
        out.camera.freeYaw = stationary.freeYaw;
        out.camera.freePitch = stationary.freePitch;
        out.camera.up = glm::dvec3(stationary.currentUp);
        // Missing transition/tour timeline cannot claim a stable BODY view.
        out.presentationMode = 0;
    }
    if (ship.active) { out.camera.mode = CAM_SPACESHIP; out.presentationMode = 0; }
}

void SaveStateManager::restoreProgression(const SimulationSaveState& state, ScienceProgression& prog) {
    if (state.version >= 3) {
        prog.applySaveData(state.progression);
    } else {
        prog.resetFresh(); // v2 and older: new-game progression, v2 data intact
    }
}

bool SaveStateManager::restoreState(const SimulationSaveState& state, SimulationController& simulation,
                                   CameraController& cam,
                                   Spaceship& ship, SolarOdysseyUI& ui) {
    if (!state.validate()) return false;
    // Prepare both world and camera before committing any live owner.
    SimulationController restored = simulation;
    if (!restored.restoreSession(state.simulationSeconds(), state.timeMultiplier, state.isPaused, state.physicsMode,
                                 state.version >= 4 ? &state.continuation : nullptr)) return false;
    CameraController restoredCamera = cam;
    restoredCamera.restoreBookmark(state.camera);
    if (state.shipActive) restoredCamera.mode = CAM_SPACESHIP;
    else if (restoredCamera.mode == CAM_SPACESHIP) restoredCamera.enterFreeCam();
    if (const auto* focused = restored.getBodyState(restoredCamera.focusedBodyName)) {
        if (state.version < 4) {
            const float radius = static_cast<float>(focused->radius) * ui.planetScale;
            restoredCamera.minFocusDistance = radius * 1.35f;
            restoredCamera.maxFocusDistance = std::max(restoredCamera.focusDistance, std::max(radius * 3.8f, 2.5f) * 4.0f);
        }
    }
    simulation = std::move(restored);
    cam = std::move(restoredCamera);
    ship.restoreSession(state);
    if (const auto* target = simulation.getBodyState(ship.targetPlanetName))
        ship.setTargetPlanet(target->name, glm::vec3(target->position), static_cast<float>(target->radius));
    if (ship.active) {
        cam.currentEye = ship.smoothCameraEye;
        cam.currentTarget = ship.smoothCameraTarget;
        cam.currentUp = ship.smoothCameraUp;
    }
    ui.elapsedSimDays = simulation.getElapsedSimDays();
    ui.timeMultiplier = static_cast<float>(state.timeMultiplier);
    ui.isPaused = state.isPaused;
    ui.physicsMode = state.physicsMode;
    ui.pendingPhysicsModeChange = false;
    ui.orbitSpeedScale = static_cast<float>(simulation.getOrbitSpeedScale());
    ui.autoSaveOnExit = state.autoSaveOnExit;
    ui.selectedPlanetName = state.version >= 4 ? state.selectedBodyName :
        (cam.mode == CAM_FOCUS || cam.mode == CAM_POV) ? cam.focusedBodyName : ship.active ? ship.targetPlanetName : "";
    return true;
}

std::string SaveStateManager::getSaveSummary(const std::string& filepath) const {
    SimulationSaveState state;
    if (!const_cast<SaveStateManager*>(this)->loadFromFile(filepath, state)) {
        return "No save file found";
    }
    std::ostringstream ss;
    ss << "Day " << std::fixed << std::setprecision(1) << state.displayDays()
       << " (" << state.timeMultiplier << "x, "
       << (state.camera.mode == CAM_FOCUS ? state.camera.focusedBodyName + " Focus" :
           state.camera.mode == CAM_FREE ? "Free Flight" :
           state.camera.mode == CAM_SPACESHIP ? "Spaceship" : "Overview")
       << ")";
    return ss.str();
}
