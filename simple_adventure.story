story {
    title: "The Enchanted Forest";
    
    inventory {
        item key {
            description: "A rusty old key";
        }
        
        item lantern {
            description: "A small oil lantern";
        }
    }
    
    room entrance {
        description: "You stand at the entrance to a dark forest. The trees loom overhead, their branches forming a canopy that blocks most of the sunlight.";
        
        choice "What do you want to do?" {
            option "Enter the forest" goto forest_path;
            option "Go back home" goto end_game;
        }
    }
    
    room forest_path {
        description: "You're on a narrow path winding through the forest. The undergrowth is thick on either side.";
        
        choice "Which way will you go?" {
            option "Continue deeper" goto clearing;
            option "Turn back" goto entrance;
        }
    }
    
    room clearing {
        description: "You've reached a small clearing in the forest. Moonlight streams down, illuminating an old wooden chest in the center.";
        
        choice "What will you do with the chest?" {
            option "Try to open it" goto chest_open;
            option "Leave it alone" goto forest_path;
        }
    }
    
    room chest_open {
        description: "You've opened the chest and found a map that seems to lead to a hidden treasure!";
        
        choice "What will you do next?" {
            option "Follow the map" goto treasure;
            option "Return to the path" goto forest_path;
        }
    }
    
    room treasure {
        description: "Following the map, you've discovered an ancient treasure hidden for centuries!";
        
        choice "What now?" {
            option "Take the treasure and go home" goto end_game;
            option "Leave it for others to find" goto forest_path;
        }
    }
    
    room end_game {
        description: "Your adventure has come to an end. Thanks for playing!";
    }
}