#include "Player.h"

Player::Player(const glm::vec3& position, float hitpoints, float moveSpeed, AssimpModel* model, const glm::vec3& scale, const glm::vec3& rotation)
    : Entity(position, hitpoints, moveSpeed, model, scale, rotation) {
    // Add any Player-specific initialization here
    std::cout << "Player Entity Created." << std::endl;
    // Initialize currentSpell to point to the first spell in the inventory as a default
    currentSpell = &spellInventory[0]; 
    activeSpellSlot = SLOT_LEFT; // Default active slot
}

SpellSlot Player::getActiveSpellSlot() {
    return activeSpellSlot;
}

void Player::setCurrentSpellSlot(SpellSlot slot) {
    activeSpellSlot = slot;
    currentSpell = &spellInventory[slot];
}

void Player::pickupSpell(Spell *spell, SpellSlot slotToPickupTo) { // Renamed activeSpellSlot to slotToPickupTo for clarity
    if (spell == nullptr) return;

    // Check if the target slot is valid
    if (slotToPickupTo >= SLOT_LEFT && slotToPickupTo < (SLOT_LEFT + Config::INVENTORY_SIZE)) {
        spellInventory[slotToPickupTo] = *spell;
        // If the picked-up spell is in the currently active slot, update currentSpell pointer
        if (activeSpellSlot == slotToPickupTo) {
            currentSpell = &spellInventory[slotToPickupTo];
        }
        std::cout << "Picked up " << spell->getName() << " into slot " << slotToPickupTo << std::endl;
    } else {
        // If no specific slot or an invalid slot is given, try to find an empty slot
        for (int i = 0; i < Config::INVENTORY_SIZE; i++) {
            if (spellInventory[i].getName() == "Empty") { // Check by name for an empty slot
                spellInventory[i] = *spell;
                std::cout << "Picked up " << spell->getName() << " into first empty slot " << i << std::endl;
                // If the active spell was an empty slot that just got filled, update currentSpell
                if (currentSpell->getName() == "Empty" && activeSpellSlot == static_cast<SpellSlot>(i)) {
                     currentSpell = &spellInventory[i];
                }
                return;
            }
        }
        // If inventory is full and no specific slot was targeted for replacement, you might log or handle this case
        std::cout << "Inventory full, could not pick up " << spell->getName() << std::endl;
    }
}

bool Player::isInventoryFull() {
    for (int i = 0; i < Config::INVENTORY_SIZE; i++) {
        if (spellInventory[i].getName() == "Empty") { // Check by name for an empty slot
            return false;
        }
    }
    return true;
}

Spell Player::getSpellFromSpellSlot(SpellSlot slot) {
    if (slot >= SLOT_LEFT && slot < (SLOT_LEFT + Config::INVENTORY_SIZE)) {
        return spellInventory[slot];
    }
    // Return a default "Empty" spell if the slot is invalid to prevent crashes
    // This requires Spell to have a constructor that can be called this way, 
    // or you handle errors differently.
    return Spell("Empty", 0,0,0,0,0, SpellType::NONE); 
}

void Player::setSprintFlag(bool flag) {
    sprintFlag = flag;
}

void Player::move(const glm::vec3& direction, float deltaTime) {
    // Assuming 'rotation' in Entity stores the orientation needed for a forward vector
    // This part might need adjustment based on how Entity::rotation is stored/used
    glm::vec3 forwardVector = glm::vec3(sin(getRotation().y), 0, cos(getRotation().y)); // Example if rotation.y is yaw
    // Or, if Entity has a method to get the forward direction:
    // glm::vec3 forwardVector = getForwardDirection(); 

    if (sprintFlag) {
        Entity::move(direction * 2.0f, deltaTime);     // arbitrary sprint speed
    } else {
        Entity::move(direction, deltaTime);
    }
}

void Player::castSpell() {
    if (currentSpell != nullptr && currentSpell->getName() != "Empty") { // Check by name and ensure pointer is valid
        // Assuming Entity has getPosition() and a way to get forward direction
        // For targetDirection, we need a forward vector. 
        // If Entity's 'rotation' is yaw, pitch, roll, we derive it.
        // Let's assume rotation.y is yaw for now, similar to Application::manMoveDir
        glm::vec3 forwardDir = glm::vec3(sin(this->getRotation().y), 0, cos(this->getRotation().y));
        currentSpell->cast(this->getPosition(), forwardDir);
        std::cout << "Casting " << currentSpell->getName() << std::endl;
    } else {
        std::cout << "Cannot cast. No valid spell selected or currentSpell is null." << std::endl;
    }
}

void Player::setDamageTimer(float timer) {
    damageTimer = timer;
}

float Player::getDamageTimer() {
    return damageTimer;
}

void Player::takeDamage(float damage) {
    if (!isAlive()) return; // Can't damage dead entities

    setDamageTimer(Config::PLAYER_HIT_DURATION);

    hitpoints -= damage;
    std::cout << "Player took " << damage << " damage. HP: " << hitpoints << std::endl;
    if (hitpoints <= 0) {
        hitpoints = 0;
        alive = false; // Set alive flag to false
        std::cout << "Player has died." << std::endl;
    }
}