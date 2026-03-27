#ifndef  __PJ_HW_DEF_H
#define  __PJ_HW_DEF_H

// #define Release 
// #ifndef Release
//   #warning "Release not defined, debug interface will be open! Pelase define Release to close debug interface!"
// #endif

//projector hardware model enum

#define Non_Model                 0x0 
#define LP72BSL_NP52SLC_NP52SL    0x1       
#define LP92SLB                   0x2       
#define NP52SL                    0x3       
#define NP72BN                    0x4       

//projector hardware model define
#define Projector_Model LP72BSL_NP52SLC_NP52SL
// #define Projector_Model LP92SLB

//Unified software version number
#define VERSION_MAJOR 2
#define VERSION_MINOR 5
#define VERSION_PATCH 4

#define MACRO_1
#define MACRO_2 


// function module define 
// #define USING_PJLINK

# if(Projector_Model== LP72BSL_NP52SLC_NP52SL)  // 6 laser model
//   #define Projector_Ver 1.90f	  
  #define ProjectorName "LP72BSL_NP52SLC_NP52SL"
  #define ProductName  "SimulationProjector"
#endif 

# if(Projector_Model== NP52SL)  // 5 laser model 
  #define ProjectorName "NP52SL"
  #define ProductName  "Simu Projector"
# endif

# if(Projector_Model == LP92SLB)
//   #define Projector_Ver 1.11f    
  #define ProjectorName "LP92SLB"
  #define ProductName  "EngineeringProjector"
//   #define USING_PJLINK
# endif

# if(Projector_Model == NP72BN)   // no use
//   #define Projector_Ver 0.1f
  #define ProjectorName "NP72BN"
# endif

# if(Projector_Model== Non_Model)
//   #define Projector_Ver 0.4f	
  #define ProjectorName "Non_Model"
# endif 		


#if (Projector_Model == NP52SL)
  #define Temp_B2 0 
#else 
  #define Temp_B2 1 
#endif 	


#define SHIFT_MOTOR_NEW_DZ_24BYJ48_A14  //New motor model: DZ_24BYJ48_A14

#endif //

