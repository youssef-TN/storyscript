// Simple StoryScript Adventure Example

// Global variables
var hasKey = false;
var visitedRooms = 0;

// Main entrance room
room Entrance {
  description: "You stand in the entrance of a mysterious cave. The walls glisten with moisture and strange symbols.";
  exits: {
    north: MainChamber
    east: null  // Locked initially
  }
  
  item Torch {
    description: "A burning torch illuminates the darkness.";
    takeable: true;
  }
  
  when entered {
    visitedRooms = visitedRooms + 1;
    say "The cool air from inside the cave sends shivers down your spine.";
  }
}

// Main chamber room
room MainChamber {
  description: "A large chamber with high ceilings. Water drips from stalactites above.";
  exits: {
    south: Entrance
    west: TreasureRoom
  }
  
  item Key {
    description: "A rusty old key lies on the ground.";
    takeable: true;
  }
  
  when entered {
    visitedRooms = visitedRooms + 1;
    say "Your footsteps echo in the vast chamber.";
    if (not hasKey and player.canSee(Key)) {
      say "You notice something glinting on the floor.";
    }
  }
}

// Treasure room
room TreasureRoom {
  description: "A small room filled with ancient artifacts and gold coins.";
  exits: {
    east: MainChamber
  }
  
  item Chest {
    description: "A large wooden chest with iron bands.";
    locked: true;
    takeable: false;
  }
  
  when entered {
    visitedRooms = visitedRooms + 1;
    say "The glint of gold fills you with excitement!";
  }
}

// Function to handle taking the key
function takeKey() {
  hasKey = true;
  say "You pick up the rusty key. It might open something important.";
  player.inventory.add(Key);
  MainChamber.removeItem(Key);
}

// Function to handle unlocking the chest
function unlockChest() {
  if (hasKey) {
    say "You use the rusty key to unlock the chest. Inside you find a magical amulet!";
    player.inventory.add(Amulet);
    hasKey = false;  // The key gets stuck in the lock
  } else {
    say "The chest is locked. You need a key.";
  }
}