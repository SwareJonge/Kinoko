#pragma once

#include "game/field/obj/ObjectCollidable.hh"

namespace Field {

class ObjectRock : public ObjectCollidable {
public:
    ///@addr {0x8076F2E0}
    ObjectRock(const System::MapdataGeoObj &params) : ObjectCollidable(params) {}
    ~ObjectRock() override;

    void init() override;
    void calc() override;

    /// @addr{0x8077037C}
    [[nodiscard]] u32 loadFlags() const override {
        return 1;
    }

    /// @addr{0x8077036C}
    [[nodiscard]] const EGG::Vector3f &getCollisionTranslation() const override {
        return m_colTranslate;
    }

    [[nodiscard]] Kart::Reaction onCollision(Kart::KartObject * kartObj,
            Kart::Reaction reactionOnKart, Kart::Reaction reactionOnObj,
            EGG::Vector3f &hitDepth) override;

    void calcTangible();
    void calcIntangible();
    void calcTangibleSub();

    void checkSphereFull();
    void breakRock(const EGG::Vector3f &hitDepth);

private:
    enum class State {
        Tangible = 0,
        Intangible = 1,
    };

    State m_state;
    f32 m_startYPos;
    EGG::Vector3f m_colTranslate;
    f32 m_angSpd;
    int m_cooldownTimer;
    f32 m_angRad;

    static constexpr f32 INITIAL_ANGULAR_SPEED = 3.0f;
};


}