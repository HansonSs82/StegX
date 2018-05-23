#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>

#include "common.h"
#include "stegx_common.h"
#include "stegx_errors.h"
#include "../insert.h"
#include "../protection.h"

int insert_eoc(info_s * infos)
{
    uint8_t byte_cpy;
    uint32_t data_size;
    uint8_t tag_type; 
    uint32_t cpt_video_tag=0;
	assert(infos);
    assert(infos->mode == STEGX_MODE_INSERT);
    assert(infos->algo == STEGX_ALGO_EOC);
    uint32_t prev_tag_size;
    uint32_t data_per_vtag=infos->hidden_length/infos->host.file_info.flv.nb_video_tag;
    uint32_t reste=infos->hidden_length%infos->host.file_info.flv.nb_video_tag;
    uint8_t *data;
    uint32_t *data2;
    int datab;
    printf("dt %u\n",data_per_vtag);
    printf("r %u\n",reste );
    printf("nbt %u\n",infos->host.file_info.flv.nb_video_tag);

    if ((fseek(infos->host.host, 0, SEEK_SET) == -1) || (fseek(infos->hidden,0,SEEK_SET) == -1)){
        return perror("Can't do insertion EOC"), 1;}
	


	
	 
    if(infos->host.file_info.flv.nb_video_tag<256){

    	data = malloc(infos->host.file_info.flv.nb_video_tag * sizeof(uint8_t));

		if (!data)
			return perror("Can't allocate memory Insertion"), 1;
		
		for(uint32_t i=0;i<infos->host.file_info.flv.nb_video_tag;i++){
			data[i]=i;
        	
        	
 		}
 	protect_data(data,infos->host.file_info.flv.nb_video_tag,infos->passwd, STEGX_MODE_INSERT);
 	datab=0;
 	} 
 	else {
 			printf("hey\n");
 			data2 = malloc(infos->host.file_info.flv.nb_video_tag * sizeof(uint32_t));
 			for(uint32_t i=0;i<infos->host.file_info.flv.nb_video_tag;i++){
				data2[i]=i;
				printf("data %u, i %u \n ",data2[i], i);

			}
			datab=1;
 		}

 
	 	//printf("%u\n",data[infos->host.file_info.flv.nb_video_tag+10]);
	

 //~ for(uint8_t i=0;i<infos->host.file_info.flv.nb_video_tag;i++){
            //~ printf("video_tag : %u  nb_blocks : %u\n",i, data[i]);
            
        //~ }
    	//recopie header
    	for(int j=0;j<13;j++){
    		if (fread(&byte_cpy, sizeof(uint8_t), 1, infos->host.host) != 1)
                return perror("Can't read Header"), 1;
            if (fwrite(&byte_cpy, sizeof(uint8_t), 1, infos->res) == 0)
                return perror("Can't write Header"), 1;
            
    	}
    	
    	
    	do{

    		//recherche video tag
    		fread(&tag_type, sizeof(uint8_t), 1, infos->host.host);
    		if (tag_type!=9){
    		    	fwrite(&tag_type, sizeof(uint8_t), 1, infos->res);
    		    	fread(&data_size, sizeof(data_size), 1, infos->host.host);
    		    	fwrite(&data_size, sizeof(uint32_t), 1, infos->res);
    		    	//passage en 24 bits      
           	  	  	data_size = be32toh(data_size) >> 8;
        			//recopie data + 6 octets
        			for(uint32_t j=0;j<data_size+6;j++){
        				fread(&byte_cpy, sizeof(uint8_t), 1, infos->host.host);
    			  		fwrite(&byte_cpy, sizeof(uint8_t), 1, infos->res);
    				}
    				//recopie prev tag size
    				fread(&prev_tag_size, sizeof(uint32_t), 1, infos->host.host);
    				fwrite(&prev_tag_size, sizeof(uint32_t), 1, infos->res);
    		}
    		else {
    			fwrite(&tag_type, sizeof(uint8_t), 1, infos->res);
    			fread(&data_size, sizeof(data_size), 1, infos->host.host);
    			 
    		
    			/* /!\ */
    			//l'octet de droite qui fait parti des datas
    			//passage en 24 bits      
            	data_size= be32toh(data_size); 
            	uint8_t tmp = data_size;
            	data_size>>= 8;

            	
            	uint32_t data_size_host=data_size;

			/* Ecriture de la nouvelle taille des datas.
			 * /!\ verifier que la taille à inscrire < 2^24
			 * cas particulier dernier tag (ou du premier si on utilise pas l'algo de protection) */
            	
            	if(infos->host.file_info.flv.nb_video_tag<256){
            		if (data[cpt_video_tag] == infos->host.file_info.flv.nb_video_tag-1)
						data_size += reste + data_per_vtag;
            		else 
						data_size += data_per_vtag;

				}
				else{
					if (data2[cpt_video_tag] == infos->host.file_info.flv.nb_video_tag-1)
					data_size += reste + data_per_vtag;
            		else 
					data_size += data_per_vtag;
					}

            	uint8_t tmp2=data_size;
            	uint8_t tmp3=data_size>>8;
            	uint8_t tmp4=data_size>>16;
            	//recopie data size en big endian

            	fwrite(&tmp4,sizeof(uint8_t),1,infos->res);
            	fwrite(&tmp3,sizeof(uint8_t),1,infos->res);
            	fwrite(&tmp2	,sizeof(uint8_t),1,infos->res);
            	fwrite(&tmp,sizeof(uint8_t),1,infos->res);
            	//copie des data d'origine + 6 octets

            	for(uint32_t j=0;j<data_size_host+6;j++){
        				fread(&byte_cpy, sizeof(uint8_t), 1, infos->host.host);
    			  		fwrite(&byte_cpy, sizeof(uint8_t), 1, infos->res);
    			 }
    			 
    			 //écriture données caché
    			

    			

    			uint32_t nb_block=(infos->host.file_info.flv.nb_video_tag<256) ?
    									data[cpt_video_tag]:data2[cpt_video_tag];
    			
    			
    		// 	printf(" nb tag%u\n",infos->host.file_info.flv.nb_video_tag);

    	 /*ici*/		printf("nb_block %u\n", nb_block);
    			printf("cpt %u\n",cpt_video_tag);
    			fseek(infos->hidden, nb_block*data_per_vtag, SEEK_SET);
    			uint32_t limit;
				limit = (nb_block == infos->host.file_info.flv.nb_video_tag-1) ? 
										data_per_vtag + reste : data_per_vtag;
				printf("limit %u\n",limit );
    			for(uint32_t i=0;i<limit;i++){
    				fread(&byte_cpy,sizeof(uint8_t),1,infos->hidden);
    				fwrite(&byte_cpy,sizeof(uint8_t),1,infos->res);

    			}
    			 //lecture previous tagsize
    			fread(&prev_tag_size,sizeof(uint32_t),1,infos->host.host);
    			prev_tag_size = be32toh(prev_tag_size);

    			// /!\ taille à verif 
    			//modif previous tag size + ecriture
    			if (nb_block == infos->host.file_info.flv.nb_video_tag-1)
					prev_tag_size += reste + data_per_vtag; //cas particulier dernier tag
            	else 
					prev_tag_size += data_per_vtag;
    			prev_tag_size = htobe32(prev_tag_size);
    			fwrite(&prev_tag_size,sizeof(uint32_t),1,infos->res);
    			cpt_video_tag += 1;
			}
        }while(cpt_video_tag < infos->host.file_info.flv.nb_video_tag);
	
	while(fread(&byte_cpy, sizeof(uint8_t), 1, infos->host.host)==1)
			fwrite(&byte_cpy,sizeof(uint8_t),1,infos->res);
		
    write_signature(infos);
    if (datab) free(data2);
    else free(data);
    return 0;
    
}


int extract_eoc(info_s * infos)
{
	uint8_t byte_cpy;
    uint32_t data_size;
    uint8_t tag_type; 
    uint32_t cpt_video_tag=0;
	uint32_t nb_block = 0;
	uint32_t data_jump;
	uint32_t write_data;
	assert(infos);
    assert(infos->mode == STEGX_MODE_EXTRACT);
    assert(infos->algo == STEGX_ALGO_EOC);

    uint32_t data_per_vtag=infos->hidden_length/infos->host.file_info.flv.nb_video_tag;
    uint32_t reste=infos->hidden_length%infos->host.file_info.flv.nb_video_tag;
    uint8_t *data;
    uint32_t *data2;
    int datab;
    uint32_t cursor = 0;


   if (fseek(infos->host.host, 0, SEEK_SET) == -1){
        return perror("Can't do extraction EOC"), 1;
	}

    /* Initialisation de protect data */
    if(infos->host.file_info.flv.nb_video_tag<256){

    	data = malloc(infos->host.file_info.flv.nb_video_tag * sizeof(uint8_t));

		if (!data)
			return perror("Can't allocate memory Insertion"), 1;
		
		for(uint32_t i=0;i<infos->host.file_info.flv.nb_video_tag;i++){
			data[i]=i;
        	
        	printf("data %u, i %u \n ",data[i], i);
 		}
 	protect_data(data,infos->host.file_info.flv.nb_video_tag,infos->passwd, STEGX_MODE_INSERT);
 	datab=0;
 	} 
 	else {
 			printf("hey\n");
 			data2 = malloc(infos->host.file_info.flv.nb_video_tag * sizeof(uint32_t));
 			for(uint32_t i=0;i<infos->host.file_info.flv.nb_video_tag;i++){
				data2[i]=i;
				printf("data %u, i %u \n ",data2[i], i);

			}
			datab=1;
 		}
	
 
		do {

				//saute de header
				fseek(infos->host.host, 13, SEEK_SET);
				cursor = 0;
				
				if(datab){
					printf("cursor %u data %u  nb_block %u\n",cursor,data2[cursor], nb_block);
					while(data2[cursor] != nb_block)
						cursor++;
					cpt_video_tag = -1;
					/* Recherche du tag vidéo numéro cursor */
					printf("%u\n",cursor);
					while(cursor != cpt_video_tag){
						fread(&tag_type, sizeof(uint8_t), 1, infos->host.host);
						if(tag_type == 9)
							cpt_video_tag++;
						fread(&data_size, sizeof(uint32_t), 1, infos->host.host);
						//passage en 24 bits      
						data_size = be32toh(data_size) >> 8;
						/* Si c'est pas le tag recherché, on jump 
						 * les 6 octets après data_size + les data + le previous tag size (4 octets) */ 
						if(cursor != cpt_video_tag)
							fseek(infos->host.host, data_size + 10, SEEK_CUR);
					}
				}
				else {
					
						while(data[cursor] != nb_block)
						cursor++;
					cpt_video_tag = -1;
					/* Recherche du tag vidéo numéro cursor */
					while(cursor != cpt_video_tag){
						fread(&tag_type, sizeof(uint8_t), 1, infos->host.host);
						if(tag_type == 9)
							cpt_video_tag++;
						fread(&data_size, sizeof(uint32_t), 1, infos->host.host);
						//passage en 24 bits      
						data_size = be32toh(data_size) >> 8;
						/* Si c'est pas le tag recherché, on jump 
						 * les 6 octets après data_size + les data + le previous tag size (4 octets) */ 
						if(cursor != cpt_video_tag)
							fseek(infos->host.host, data_size + 10, SEEK_CUR);
					}






				}
			/* Calcul le nombre d'octets à sauter pour arriver avant les données à récupérer */
			if (nb_block == infos->host.file_info.flv.nb_video_tag - 1){
				data_jump = data_size - reste - data_per_vtag + 6; //cas particulier dernier tag
				write_data = reste + data_per_vtag;
			}
			else {
				data_jump = data_size - data_per_vtag + 6;
				write_data = data_per_vtag;
			}
			fseek(infos->host.host, data_jump, SEEK_CUR);
			/* Recopie des données dans le fichhier resultat */
			for(uint32_t i = 0; i < write_data; i++){
				fread(&byte_cpy, sizeof(uint8_t), 1, infos->host.host);
				fwrite(&byte_cpy,sizeof(uint8_t),1,infos->res);
			}
			
			nb_block++;
		}while(nb_block < infos->host.file_info.flv.nb_video_tag);
	if (datab)free(data2);
	else free(data);
	return 0;

}
