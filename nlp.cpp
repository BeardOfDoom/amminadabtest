/**
 * @brief JUDAH - Jacob is equipped with a text-based user interface
 *
 * @file nlp.cpp
 * @author  Norbert Bátfai <nbatfai@gmail.com>
 * @version 0.0.1
 *
 * @section LICENSE
 *
 * Copyright (C) 2015 Norbert Bátfai, batfai.norbert@inf.unideb.hu
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * @section DESCRIPTION
 *
 * JACOB, https://github.com/nbatfai/jacob
 *
 * "The son of Isaac is Jacob." The project called Jacob is an experiment
 * to replace Isaac's (GUI based) visual imagination with a character console.
 *
 * ISAAC, https://github.com/nbatfai/isaac
 *
 * "The son of Samu is Isaac." The project called Isaac is a case study
 * of using deep Q learning with neural networks for predicting the next
 * sentence of a conversation.
 *
 * SAMU, https://github.com/nbatfai/samu
 *
 * The main purpose of this project is to allow the evaluation and
 * verification of the results of the paper entitled "A disembodied
 * developmental robotic agent called Samu Bátfai". It is our hope
 * that Samu will be the ancestor of developmental robotics chatter
 * bots that will be able to chat in natural language like humans do.
 *
 */

 #include "nlp.hpp"
 #include <regex>

 // #define DEBUG

 SPOTriplets NLP::sentence2triplets ( const char* sentence )
 {
   // vector of triplets
   SPOTriplets triplets;

   #ifdef DEBUG
     std::cout << "The sentence: " << sentence << std::endl;
   #endif
   // creates a Sentence from the input char*
   Sentence sent = sentence_create ( sentence, dict_ );
   #ifdef DEBUG
     std::cout << "Sentence created" << std::endl;
   #endif
   // tokenizes the sentence
   sentence_split ( sent, parse_opts_ );
   #ifdef DEBUG
     std::cout << "Sentence splitted" << std::endl;
   #endif
   // searches for all possible linkages
   int num_linkages = sentence_parse ( sent, parse_opts_ );
   
   if(num_linkages == 0)
   {
     parse_options_set_min_null_count(parse_opts_, 1);
     num_linkages = sentence_parse ( sent, parse_opts_ );
   }
   
   #ifdef DEBUG
     std::cout << "Sentence parsed" << std::endl;
     std::cout << "Number of linkages: " << num_linkages << std::endl;
   #endif

   // just one triplet
   SPOTriplet triplet;

   // if there is any linkage in the sentence
   if( num_linkages > 0 )
   {
     // create the linkage
     Linkage linkage = linkage_create ( 0, sent, parse_opts_ );

     #ifdef DEBUG
       // prints the sentence's diagram
       std::cout << "The diagram: " << std::endl;
       char *diagram = linkage_print_diagram(linkage, true, 800);
       std::cout << diagram << std::endl;
       linkage_free_diagram( diagram );
       // end print diagram
     #endif

     std::vector<std::string> labels;
     
     // label-ek kinyerése
     for(int i = 0; i < linkage_get_num_links(linkage); i++ ) {
       labels.push_back(linkage_get_link_label(linkage,i));
     }

     // 1. find the S_link
     // S* except there is an SJ* because then S* except Spx
     // two cases: there is SJ* and there is not SJ*

     // TODO: VJlp VJrp same as SJ but to predications
     // TODO: SFut SFst what the fuck?                                     ###FIXED###
     // TODO: His form was shining like the light not working              ###FIXED###
     // TODO: Car is mine not working                                      ###FIXED###
     // TODO: The little brown bear has eaten all of the honey not working ###FIXED###

     // REGEXES
     std::regex SJ_( "SJ.*" );
     std::regex VJ_( "VJ.*");
     std::regex subject( "(S.*)|(Ss.*)|(SFut)|(Sp\*.*)" );
     std::regex Spx( "Spx.*" );
     // TODO:fix theese initializer list not allowed                       ###FIXED###
     std::regex predicate( "(Pv.*)|(Pg.*)|(PP.*)|(I.*)|(TO)|(MVi.*)" );
     // TODO: make one from theese // (Sp.*)|(Ss.*)                        ###FIXED###
     std::regex noun_adject_object ( "(O.*)|(Os.*)|(Op.*)|(MVpn.*)|(Pa.*)|(MVa.*)" );
     std::regex preposition ( "(MVp.*)|(Pp.*)|(OF)|(TO)" );
     std::regex prep_object ( "(J.*)|(TI)|(I.*)|(ON)" );
     // TODO: problems with matching!! Pg*!!                               ###FIXED###
     // TODO: problems with matching!! Mvp.*!!                             ###FIXED###

     bool s_found = false;
     bool p_found = false;
     bool o_found = false;
     bool SJ = false;

     // search for SJ.s labels
     for( auto label: labels )
     {
       if( std::regex_match( label, SJ_ ) )
       {
         SJ = true;
         break;
       }
     }

     // multiple subject in the sentence
     if( SJ )
     {
       // SPls left -> first subject
       // SPrs right -> second subject
       // Spx right -> predicate
       // SJ-s are multiple subjects
       std::string temp;
       // go through every linkage
       for( int i = 0; i < linkage_get_num_links( linkage ); ++i )
       {
         // get their label
         std::string l = linkage_get_link_label( linkage, i );
         // if there is an SJl* label
         if( std::regex_match( l, std::regex( "SJl.*" ) ) )
         {
           // SJls left side
           triplet.o = linkage_get_word( linkage, linkage_get_link_lword( linkage, i ) );
           triplet.cut( triplet.o );
           temp = triplet.o + " ";
           // and word
           triplet.o = linkage_get_word( linkage, linkage_get_link_rword( linkage, i ) );
           triplet.cut( triplet.o );
           temp += triplet.o + " ";

           // find SJr*
           for( int j = 0; j < linkage_get_num_links( linkage ); ++j )
           {
             std::string m = linkage_get_link_label( linkage, j );
             if( std::regex_match( m, std::regex( "SJr.*" ) ) )
             {
               triplet.o = linkage_get_word( linkage, linkage_get_link_rword( linkage, j ) );
               triplet.cut();
               temp += triplet.o;
               triplet.o = temp;

               o_found = true;
               #ifdef DEBUG
                 std::cout << "Object found: " << triplet.o << std::endl;
               #endif
               break;
             } // if
           } // for
           break;
         } // if
       } // for

       // now we have the subject

       // find Spx and its right side will be the starter predicate
       std::string current_word;
       for( int i = 0; i < linkage_get_num_links( linkage ); ++i )
       {
         std::string l = linkage_get_link_label( linkage, i );
         if( std::regex_match( l, std::regex( "Spx.*" ) ) )
         {
           triplet.p = linkage_get_word( linkage, linkage_get_link_rword( linkage, i ) );
           current_word = linkage_get_word( linkage, linkage_get_link_rword( linkage, i ) );
         }
       }
       // from now all the same as on the else branch !!!!

       bool predicate_match = false;

       // search for the linkage that has triplet.s as left!
       do
       {
         predicate_match = false;

         for( int i = 0; i < linkage_get_num_links( linkage ); ++i )
         {
           // every linkage's left word
           std::string word_i = linkage_get_word( linkage, linkage_get_link_lword( linkage, i ) );
           // every linkage's label
           std::string l = linkage_get_link_label( linkage, i );

           if( std::regex_match( l, predicate ) && word_i == current_word )
           {
             // found predicate
             triplet.p = linkage_get_word( linkage, linkage_get_link_rword( linkage, i ) );
             current_word = triplet.p;
             predicate_match = true;
             break;
           }
         }
       }
       while( predicate_match );

       // we now have the predicate too
       // TODO: multiple predicates!
       p_found = true;
       #ifdef DEBUG
         std::cout << "Predicate found: " << triplet.p << std::endl;
       #endif

       // ###COPY BEGIN###

       // search for noun object or adjective object
       for( int i = 0; i < linkage_get_num_links( linkage ); ++i )
       {
         // get every linkage label
         std::string l = linkage_get_link_label( linkage, i );
         // get the left word of every linkage
         std::string l_word = linkage_get_word( linkage, linkage_get_link_lword( linkage, i ) );
         // if thete is a label that match AND its left word is the predicate
         if( std::regex_match( l, noun_adject_object ) && triplet.p == l_word && !o_found )
         {
           // then the object is that linkage's right word
           triplet.o = linkage_get_word( linkage, linkage_get_link_rword( linkage, i ) );
           triplet.cut( triplet.o );
           o_found = true;
           #ifdef DEBUG
             std::cout << "Adjective or noun object found: " << triplet.o << std::endl;
           #endif
         } // if
       } // for

       // still not found object, then search for preposition
       if( !o_found )
       {
         // go through every linkage
         for( int i = 0; i < linkage_get_num_links( linkage ); ++i )
         {
           // get the linkage's label
           std::string l = linkage_get_link_label( linkage, i );
           // and left word
           std::string word_i = linkage_get_word( linkage, linkage_get_link_lword( linkage, i ) );
           // if there is a linkage which is a preposition and its left word is the predicate
           if( std::regex_match( l, preposition ) && triplet.p == word_i )
           {
             // found preposition
             // search for prep_object
             // then the temp will contain the preposition label's right word
             std::string temp = linkage_get_word( linkage, linkage_get_link_rword( linkage, i ) );
             #ifdef DEBUG
               std::cout << "Preposition found! and its rigth word is: " << temp << std::endl;
             #endif

             for( int j = 0; j < linkage_get_num_links( linkage ); ++j )
             {
               // every linkages
               std::string m = linkage_get_link_label( linkage, j );
               // every left word
               std::string word_j = linkage_get_word( linkage, linkage_get_link_lword( linkage, j ) );

               // if there is a label with match and its left is exactly the preposition's right
               if( std::regex_match( m, prep_object ) && temp == word_j )
               {
                 triplet.o = linkage_get_word( linkage, linkage_get_link_lword( linkage, j ) );
                 triplet.cut(triplet.o);

                 triplet.o += " ";
                 // save o
                 std::string temp = triplet.o;

                 triplet.o = linkage_get_word( linkage, linkage_get_link_rword( linkage, j ) );
                 triplet.cut(triplet.o);
                 temp += triplet.o;

                 triplet.o = temp;
                 o_found = true;
                 #ifdef DEBUG
                   std::cout << "Object found: " << triplet.o << std::endl;
                 #endif
               } // if( std::regex_match( m, prep_object ) && temp == word_j ) END
             } // for J END
           } // if( std::regex_match( l, preposition ) && triplet.p == word_i ) END
         } // for I END
       } // if( !o_found ) END
       
       // subject keresése
       if(!s_found){
	
	for(int i=0; i<linkage_get_num_links(linkage); i++){
	    
	  std::string l = linkage_get_link_label(linkage, i);
	  
	  if(std::regex_match(l, subject)){
	      
	    triplet.s = linkage_get_word(linkage, linkage_get_link_lword(linkage, i));
	    triplet.cut(triplet.s);
	    
	    s_found = true;
	    break;
	    
	  }
	  
	}
	 
       }

       if( s_found && p_found && o_found )
       {
         // TODO: cut the words itself not the whole triplet
         // have to cut every word itself
         // triplet.cut();
         triplet.cut(triplet.s);
         triplet.cut(triplet.p);
         triplets.push_back( triplet );
         s_found = false;
         p_found = false;
         o_found = false;
       }
       // ###COPY END###
     }
     else // only one subject
     {
       // except Spx!!!
       // S left -> subject
       // S right -> predicate at first
       // if the word next to S right, is an element of Pv*, Pg* PP*, I*, TO, MVi*
       // then the new predicate will be that word

       std::string current_word;

       // search for subject (S_link)
       for( int i = 0; i < linkage_get_num_links( linkage ); ++i )
       {
         // get the linkage's label
         std::string l = linkage_get_link_label( linkage, i );

         if( std::regex_match( l, subject ) )
         {
           // subject found
           triplet.s = linkage_get_word( linkage, linkage_get_link_lword( linkage, i ) );
           s_found = true;
           current_word = linkage_get_word( linkage, linkage_get_link_rword( linkage, i ) );
           triplet.p = current_word;
           #ifdef DEBUG
             std::cout << "Subject found: " << triplet.s << std::endl;
           #endif
           break;
         }
       }

       if( s_found )
       {
         bool predicate_match = false;

         // search for the linkage that has triplet.s as left!
         do
         {
           predicate_match = false;

           for( int i = 0; i < linkage_get_num_links( linkage ); ++i )
           {
             // every linkage's left word
             std::string l_word = linkage_get_word( linkage, linkage_get_link_lword( linkage, i ) );
             // every linkage's label
             std::string l = linkage_get_link_label( linkage, i );

             if( std::regex_match( l, predicate ) && l_word == current_word )
             {
               // found predicate
               triplet.p = linkage_get_word( linkage, linkage_get_link_rword( linkage, i ) );
               current_word = triplet.p;
               predicate_match = true;
               break;
             }
           } // for END
         } while( predicate_match );

         p_found = true;
         #ifdef DEBUG
           std::cout << "Predicate found: " << triplet.p << std::endl;
         #endif
       } // if( s_found ) END

       // subject and predicate found
       // search for object

       // from k to linkage_get_num_links( linkage )
       // if there is any of the noun, adjective od preposition object then that
       // label's right will give the object.

       // !!! search only between labels that has triplet.p as left word !!!!!

       // search for noun object or adjective objects
       // go through all links
       for( int i = 0; i < linkage_get_num_links( linkage ); ++i )
       {
         // get every linkage label
         std::string l = linkage_get_link_label( linkage, i );
         // get the left word of every linkage
         std::string word_i = linkage_get_word( linkage, linkage_get_link_lword( linkage, i ) );
         // if thete is a label that match AND its left word is the predicate
         if( std::regex_match( l, noun_adject_object ) && triplet.p == word_i )
         {
           // then the object is that linkage's right word
           triplet.o = linkage_get_word( linkage, linkage_get_link_rword( linkage, i ) );
           o_found = true;
           triplet.cut(triplet.o);
           #ifdef DEBUG
             std::cout << "Adjective or noun object found: " << triplet.o << std::endl;
           #endif
         } // if END
       } // for END

       // still not found object, then search for preposition
       if( !o_found )
       {
         // go through every linkage
         for( int i = 0; i < linkage_get_num_links( linkage ); ++i )
         {
           // get the linkage's label
           std::string l = linkage_get_link_label( linkage, i );
           // and left word
           std::string word_i = linkage_get_word( linkage, linkage_get_link_lword( linkage, i ) );

           // if there is a linkage which is a preposition and its left word is the predicate
           if( std::regex_match( l, preposition ) && triplet.p == word_i )
           {
             // found preposition
             // search for prep_object
             // then the temp will contain the preposition label's right word
             std::string temp = linkage_get_word( linkage, linkage_get_link_rword( linkage, i ) );
             #ifdef DEBUG
               std::cout << "Preposition found! and its rigth word is: " << temp << std::endl;
             #endif

             // start search from there
             for( int j = 0; j < linkage_get_num_links( linkage ); ++j )
             {
               // every linkages
               std::string m = linkage_get_link_label( linkage, j );
               // every left word
               std::string word_j = linkage_get_word( linkage, linkage_get_link_lword( linkage, j ) );
               #ifdef DEBUG
                 if( std::regex_match( m, prep_object ) )
                     std::cout << m << " DOES match to (J.*)|(TI)|(I.*)|(ON)" << std::endl;
               #endif

               // if there is a label with match and its left is exactly the preposition's right
               if( std::regex_match( m, prep_object ) && temp == word_j )
               {
                 triplet.o = linkage_get_word( linkage, linkage_get_link_lword( linkage, j ) );
                 triplet.cut(triplet.o);

                 triplet.o += " ";
                 // save o
                 std::string temp = triplet.o;

                 triplet.o = linkage_get_word( linkage, linkage_get_link_rword( linkage, j ) );
                 triplet.cut(triplet.o);
                 temp += triplet.o;

                 triplet.o = temp;
                 #ifdef DEBUG
                   std::cout << "Object found: " << triplet.o << std::endl;
                 #endif
                 o_found = true;
               }
             } // for
           } // if
         } // for
       } // if( o_found ) END

       if( s_found && p_found && o_found )
       {
         // TODO: cut the words itself not the whole triplet ###FIXED###
         // have to cut every word itself
         // triplet.cut();

         triplet.cut(triplet.s);
         triplet.cut(triplet.p);
         triplets.push_back( triplet );
         s_found = false;
         p_found = false;
         o_found = false;
       }

     } // end else

     linkage_delete ( linkage );
   } // if( num_linkages > 0 ) END

   sentence_delete ( sent );
   return triplets;
 }
