
import SVMTK as svm




if __name__ == "__main__":
   surf1 = svm.Surface() 
   surf2 = svm.Surface() 

   surf1.make_sphere(-1.0,0.0,0.0,3.0,0.3) 
   surf2.make_sphere(1.0,0.0,0.0,3.0,0.3) 
   surf1.save("surface_overlapp_before1.off")
   surf2.save("surface_overlapp_before2.off")

   svm.separate_overlapping_surfaces(surf1,surf2) 
   svm.separate_close_surfaces(surf1,surf2) 
   surf1.save("surface_overlapp_after1.off")
   surf2.save("surface_overlapp_after2.off")
