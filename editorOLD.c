
// void UpdateAndDrawApp2(AppState *state, MyInput *input)
// {
//     if (input->keysPressed['I'] && state->editMode == EditorMode_Normal)
//     {
//         state->editMode = EditorMode_Insert;
//     }
//     else if (input->keysPressed[VK_ESCAPE] && state->editMode == EditorMode_Insert)
//     {
//         state->editMode = EditorMode_Normal;
//     } 
//     else if (state->editMode == EditorMode_Insert)
//     {
//         if(input->charEventsThisFrameCount > 0)
//             state->isFileSaved = 0;

//         for (int i = 0; i < input->charEventsThisFrameCount; i++)
//         {
//             i32 charEvent = input->charEventsThisFrame[i];
//             if (charEvent == '\r')
//                 InsertCharUnderCursor(state, '\n');
//             else if (charEvent == VK_BACKSPACE)
//                 RemoveCharLeftFromCursor(state);
//             else
//                 InsertCharUnderCursor(state, charEvent);
//         }
//     }
//     else if (state->editMode == EditorMode_Normal)
//     {
//         if (input->keysPressed['L'])
//         {
//             if(state->cursorPos < textBuffer.length - 2)
//                 state->cursorPos++;
//         }

//         if (input->keysPressed['H'] && state->cursorPos > 0)
//         {
//             state->cursorPos--;
//         }

//         if (input->keysPressed['J'])
//         {
//             i32 currentLine = GetCurrentLineIndex(state->cursorPos);
//             if(currentLine < newLinesCount - 1)
//             {
//                 i32 currentLineStart = 0;
//                 GetLineInfo(&currentLineStart, 0, currentLine);

//                 i32 nextLineStart = 0;
//                 i32 nextLineEnd = 0;
//                 GetLineInfo(&nextLineStart, &nextLineEnd, currentLine + 1);

//                 i32 diff = state->cursorPos - currentLineStart;
//                 if(nextLineEnd - nextLineStart < diff)
//                     state->cursorPos = nextLineEnd;
//                 else 
//                     state->cursorPos = nextLineStart + diff;
//             }
//         }

//         if (input->keysPressed['K'])
//         {
//             i32 currentLine = GetCurrentLineIndex(state->cursorPos);
//             if(currentLine > 0)
//             {
//                 i32 currentLineStart = 0;
//                 GetLineInfo(&currentLineStart, 0, currentLine);

//                 i32 prevLineStart = 0;
//                 i32 prevLineEnd = 0;
//                 GetLineInfo(&prevLineStart, &prevLineEnd, currentLine - 1);

//                 i32 diff = state->cursorPos - currentLineStart;
//                 if(prevLineEnd - prevLineStart < diff)
//                     state->cursorPos = prevLineEnd;
//                 else 
//                     state->cursorPos = prevLineStart + diff;
//             }
//         }

//         if(input->keysPressed['S'] && input->isPressed[VK_CONTROL])
//         {
//             WriteMyFile(filePath, textBuffer.text, textBuffer.length);
//             state->isFileSaved = 1;
//         }

        
//     }

    

//     FontData * font = &state->fonts.regular;

//     i32 textX = 20;
//     i32 textY = 20;

//     i32 lineNumber = GetCurrentLineIndex(state->cursorPos);
//     i32 prevLineEnd = lineNumber == 0 ? 0 : newLines[lineNumber - 1] + 1;
//     i32 charPositionInLine = state->cursorPos -  prevLineEnd;

//     //Cursor Monospaced position
//     i32 symbolWidth = GetGlyphWidth(font, 'i');
//     // i32 cursorX = textX + GetTextWidth
//     // i32 cursorY = textY + lineNumber * (font->textMetric.tmHeight + 4);
//     // u32 color = state->editMode == EditorMode_Normal ? 0x0059D1 : 0xaa22aa;
//     // DrawRect(&state->canvas, cursorX - 1, cursorY, 2, font->textMetric.tmHeight, color);

//     for (int i = 0; i <= newLinesCount; i++)
//     {
//         i32 prevLine = i == 0 ? 0 : newLines[i - 1] + 1;
//         i32 line = newLines[i];

//         DrawTextLeftTop(&state->canvas, font, textX, textY, textBuffer.text + prevLine, line - prevLine, 0xeeeeee);
        
//         textY += font->textMetric.tmHeight + 4;
//     }


//     // Drawing status bar at the bottom
//     i32 labelsC = 0x888888;
//     i32 padding = 10;
//     char *label = state->editMode == EditorMode_Normal ? "Normal" : "Insert";
//     DrawTextLeftBottom(&state->canvas, font, padding, state->canvas.height - padding, label, strlen(label), labelsC);
    
    
    
//     // Drawing status bar at the bottom
//     char *savedLabel = state->isFileSaved ? "Saved" : "Modified";
//     i32 savedLabelX = state->canvas.width / 2 - strlen(savedLabel) / 2 * symbolWidth;
//     i32 savedLabelY = state->canvas.height - padding;
//     DrawTextLeftBottom(&state->canvas, font, savedLabelX, savedLabelY, savedLabel, strlen(savedLabel), labelsC);
// }

