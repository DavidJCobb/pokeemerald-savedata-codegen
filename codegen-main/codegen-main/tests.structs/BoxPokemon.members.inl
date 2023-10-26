   u32 personality;
   u32 otId;
   u8 nickname[11];
   u8 language;
   u8 isBadEgg : 1;
   u8 hasSpecies : 1;
   u8 isEgg : 1;
   u8 unused : 5;
   u8 otName[8];
   u8 markings;
   u16 checksum;
   u16 unknown;
   union {
      u32 raw[(NUM_SUBSTRUCT_BYTES * 4) / 4];
      union PokemonSubstruct substructs[4];
} secure;
